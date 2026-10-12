/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kernel_memtracker.c
 * @par Purpose:
 *     OS tracks which parts of memory change. Allows efficient diffs.
 * @par Comment:
 *     Linux: soft-dirty bits (/proc/self/clear_refs; /proc/self/pagemap).
 *     Windows: page fault + vectored exception handler
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kernel_memtracker.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#define KMT_WINDOWS 1
#elif defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#define KMT_LINUX 1
#endif
#include "globals.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct KmtRange {
    unsigned char *ptr; // page-aligned
    size_t len; // multiple of page size
    size_t first_chunk;
};

static struct KmtRange ranges[KMT_MAX_RANGES];
static unsigned chunk_bitshift;
static int ranges_count;
static size_t total_chunks;
static TbBool available;

#define KMT_MIN_CHUNKSIZE 4096
static TbBool armed;

/******************************************************************************/
#if defined(KMT_LINUX)

/* On Linux x86-64 kernel, a "soft dirty bit" (sdb) is tracked for
 * each page, which tracks "has changed since last time sdb cleared"
 *
 * FIXME: This approach doesn't work on ARM, nor 32-bit x86
 */
static int clear_fd = -1;
static int map_fd = -1;

// buffer long enough to read all page descriptors for the longest chunk.
static uint64_t *map_buf;
static size_t map_buf_count;

#define SOFT_DIRTY_BIT (1ULL << 55)

static TbBool clear_soft_dirty(void)
{
    // magic system call to clear all soft dirty bits
    return (
        write(clear_fd, "4", 1) == 1
    );
}

static TbBool read_soft_dirty(const void *addr, TbBool *dirty)
{
    uint64_t entry;
    const off_t off = (off_t)((uintptr_t)addr / kernel_memtracker_chunk_size()) * sizeof(entry);
    if (pread(map_fd, &entry, sizeof(entry), off) != (ssize_t)sizeof(entry))
        return false;
    *dirty = !!(entry & SOFT_DIRTY_BIT);
    return true;
}

extern char __bss_start[];
extern char _end[];

// run before main() to ensure bss is not irreversibly hugepaged
__attribute__((constructor))
static void disable_hugepages(void)
{
    // disable Huge Pages, which breaks our 4kb-page assumption.
    const uintptr_t ps = sysconf(_SC_PAGESIZE);
    const uintptr_t start = (uintptr_t)__bss_start / ps * ps;
    const uintptr_t end = ((uintptr_t)_end + ps - 1) / ps * ps;
    madvise((void *)start, end - start, MADV_NOHUGEPAGE);
}

static size_t platform_page_size(void)
{
    const long ps = sysconf(_SC_PAGESIZE);
    return (ps > 0) ? (size_t)ps : 0;
}

static TbBool platform_init(void)
{
    // pagemap has one entry per page, so a chunk must be exactly one page
    const size_t chunk = kernel_memtracker_chunk_size();
    if (platform_page_size() != chunk)
        return false;
    clear_fd = open("/proc/self/clear_refs", O_WRONLY);
    map_fd = open("/proc/self/pagemap", O_RDONLY);
    if ((clear_fd < 0) || (map_fd < 0))
        return false;

    // confirm it all works
    volatile unsigned char *test = (volatile unsigned char *)mmap(NULL, chunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ((void *)test == MAP_FAILED)
        return false;
    test[0] = 1;
    TbBool before = true;
    TbBool after = false;
    TbBool ok = clear_soft_dirty() && read_soft_dirty((const void *)test, &before);
    test[0] = 2;
    ok = ok && read_soft_dirty((const void *)test, &after);
    munmap((void *)test, chunk);
    return ok && !before && after;
}

static TbBool platform_add_range(struct KmtRange *rng)
{
    // disable hugepages
    madvise(rng->ptr, rng->len, MADV_NOHUGEPAGE);

    // resize map_buf to be able to store all page descriptors for this chunk
    const size_t pages = rng->len / kernel_memtracker_chunk_size();
    if (pages > map_buf_count)
    {
        uint64_t *buf = (uint64_t *)realloc(map_buf, pages * sizeof(uint64_t));
        if (buf == NULL)
            return false;
        map_buf = buf;
        map_buf_count = pages;
    }
    return true;
}

static void platform_clear(void)
{
    armed = clear_soft_dirty();
}

static TbBool platform_collect(unsigned char *dirty)
{
    // read dirty bits for all pages in all chunks
    const size_t chunk = kernel_memtracker_chunk_size();
    for (int r = 0; r < ranges_count; r++)
    {
        const struct KmtRange *rng = &ranges[r];
        const size_t pages = rng->len / chunk;
        const off_t off = (off_t)((uintptr_t)rng->ptr / chunk) * sizeof(uint64_t);
        if (pread(map_fd, map_buf, pages * sizeof(uint64_t), off) != (ssize_t)(pages * sizeof(uint64_t)))
            return false;
        for (size_t pg = 0; pg < pages; pg++)
            dirty[rng->first_chunk + pg] = !!(map_buf[pg] & SOFT_DIRTY_BIT);
    }
    return true;
}

static void platform_disarm(void)
{
    armed = false;
}
/******************************************************************************/
#elif defined(KMT_WINDOWS)

static volatile unsigned char *win_dirty;
static PVOID veh_handle;

// fires when a protected chunk is written to
static LONG CALLBACK memtracker_exception_handler(PEXCEPTION_POINTERS info)
{
    // filter out other kinds of exceptions
    const EXCEPTION_RECORD *er = info->ExceptionRecord;
    if ((er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) || (er->NumberParameters < 2) || (er->ExceptionInformation[0] != 1))
        return EXCEPTION_CONTINUE_SEARCH;

    unsigned char *const addr = (unsigned char *)er->ExceptionInformation[1];
    for (int r = 0; r < KMT_MAX_RANGES; r++)
    {
        const struct KmtRange *rng = &ranges[r];
        if ((addr < rng->ptr) || (addr >= rng->ptr + rng->len))
            continue;
        const size_t chunk = (size_t)(addr - rng->ptr) >> chunk_bitshift;
        size_t size = 1ULL << chunk_bitshift;

        // unprotect page and record dirtiness
        DWORD old;
        if (!VirtualProtect(rng->ptr + chunk * size, size, PAGE_READWRITE, &old))
            return EXCEPTION_CONTINUE_SEARCH;
        win_dirty[rng->first_chunk + chunk] = 1;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static size_t platform_page_size(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

static TbBool platform_init(void)
{
    memset(ranges, 0, sizeof(ranges));
    // for efficiency, the exception handler unprotects one chunk at a time,
    // so a chunk must be exactly one page
    if (platform_page_size() != kernel_memtracker_chunk_size())
        return false;

    veh_handle = AddVectoredExceptionHandler(1, memtracker_exception_handler);
    return (veh_handle != NULL);
}

static TbBool range_is_static_image(const unsigned char *ptr, size_t len)
{
    const unsigned char *pos = ptr;
    while (pos < ptr + len)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(pos, &mbi, sizeof(mbi)) != sizeof(mbi))
            return false;
        if ((mbi.Type != MEM_IMAGE) || (mbi.State != MEM_COMMIT) || ((mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY)) == 0))
            return false;
        pos = (const unsigned char *)mbi.BaseAddress + mbi.RegionSize;
    }
    return true;
}

static TbBool platform_add_range(struct KmtRange *rng)
{
    if (!range_is_static_image(rng->ptr, rng->len))
    {
        ERRORLOG("Memory at %p (%u bytes) is not static; cannot do kernel-accelerated memtracking", (void *)rng->ptr, (unsigned)rng->len);
        return false;
    }
    unsigned char *buf = (unsigned char *)realloc((void *)win_dirty, total_chunks + rng->len / kernel_memtracker_chunk_size());
    if (buf == NULL)
        return false;
    win_dirty = buf;
    return true;
}

static void platform_disarm(void)
{
    for (int r = 0; r < ranges_count; r++)
    {
        DWORD old;
        VirtualProtect(ranges[r].ptr, ranges[r].len, PAGE_READWRITE, &old);
    }
    armed = false;
}

static void platform_clear(void)
{
    memset((void *)win_dirty, 0, total_chunks);
    armed = true;
    for (int r = 0; r < ranges_count; r++)
    {
        DWORD old;
        if (!VirtualProtect(ranges[r].ptr, ranges[r].len, PAGE_READONLY, &old))
        {
            platform_disarm();
            return;
        }
    }

    memset(ranges, 0, sizeof(ranges));
}

static TbBool platform_collect(unsigned char *dirty)
{
    memcpy(dirty, (const void *)win_dirty, total_chunks);
    return true;
}
#endif
/******************************************************************************/

TbBool kernel_memtracker_init(void)
{
    static TbBool init_done = false;
    if (!init_done)
    {
        init_done = true;
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
        available = platform_init();
#endif
        if (!available)
        {
            WARNLOG("Kernel memory tracking is not available; falling back to scanning");
        }
    }
    return available;
}

size_t kernel_memtracker_chunk_size(void)
{
    static size_t size;
    if (size == 0)
    {
        size = KMT_MIN_CHUNKSIZE;
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
        while (size < platform_page_size())
            size *= 2;
#endif
        chunk_bitshift = __builtin_ctzll(size);
    }
    return size;
}

TbBool kernel_memtracker_add(void *ptr, size_t len)
{
    const size_t size = kernel_memtracker_chunk_size();
    if (!available || (len == 0) || ((uintptr_t)ptr % size != 0) || (len % size != 0))
        return false;
    if (ranges_count >= KMT_MAX_RANGES)
        return false;
    kernel_memtracker_disarm();
    struct KmtRange *rng = &ranges[ranges_count];
    rng->ptr = (unsigned char *)ptr;
    rng->len = len;
    rng->first_chunk = total_chunks;
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
    if (!platform_add_range(rng))
        return false;
#endif
    ranges_count++;
    total_chunks += len / size;
    return true;
}

size_t kernel_memtracker_get_chunk_count(void)
{
    return total_chunks;
}

void kernel_memtracker_clear(void)
{
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
    if (available && (ranges_count > 0))
        platform_clear();
#endif
}

TbBool kernel_memtracker_collect(unsigned char *dirty)
{
    if (!available || !armed)
        return false;
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
    return platform_collect(dirty);
#else
    return false;
#endif
}

void kernel_memtracker_disarm(void)
{
#if defined(KMT_LINUX) || defined(KMT_WINDOWS)
    if (available && armed)
        platform_disarm();
#endif
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
