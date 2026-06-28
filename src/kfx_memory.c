#include "pre_inc.h"
#include "kfx_memory.h"
#include "post_inc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declaration -- avoids dragging in PlatformManager.h and its
 * C++-only type dependencies (TbFileHandle, TbBool, etc.) into C code. */
// extern size_t PlatformManager_GetScratchSize(void);

/* ===== OOM handler ===== */
static void kfx_oom(size_t size, const char* file, int line)
{
    fprintf(stderr, "KfxAlloc: OUT OF MEMORY: %lu bytes at %s:%d\n",
            (unsigned long)size, file ? file : "?", line);
    fflush(stderr);
    abort();
}

/* ===================================================================
 * RELEASE build -- thin wrappers, zero overhead
 * =================================================================== */
#ifndef KFX_DEBUG_MEMORY

void* KfxAlloc(size_t size)
{
    void* p = malloc(size);
    if (!p && size) kfx_oom(size, NULL, 0);
    return p;
}

void* KfxCalloc(size_t count, size_t size)
{
    void* p = calloc(count, size);
    if (!p && count && size) kfx_oom(count * size, NULL, 0);
    return p;
}

void* KfxRealloc(void* ptr, size_t size)
{
    void* p;
    if (size == 0) { free(ptr); return NULL; }
    p = realloc(ptr, size);
    if (!p) kfx_oom(size, NULL, 0);
    return p;
}

void KfxFree(void* ptr)
{
    free(ptr);
}

char* KfxStrDup(const char* s)
{
    char* p;
    size_t len;
    if (!s) return NULL;
    len = strlen(s) + 1;
    p = (char*)malloc(len);
    if (!p) kfx_oom(len, NULL, 0);
    memcpy(p, s, len);
    return p;
}

void KfxMemDump(void) { /* no-op in release */ }
void KfxMemValidate(void) { /* no-op in release */ }

#else /* KFX_DEBUG_MEMORY ============================================
 * DEBUG build -- per-callsite accounting + 8-byte guard-zone canaries
 * =================================================================== */

#define KFX_MAX_SITES  256
#define KFX_MAX_ALLOCS 2048

/* Guard-zone layout for every allocation:
 *   [8-byte prefix: user_size as size_t] [user_size bytes] [8-byte suffix]
 * Both prefix and suffix (excluding the stored size) are filled with 0xFD.
 * On free, canaries are validated; corruption aborts immediately. */
#define KFX_GUARD_SIZE   8
#define KFX_GUARD_BYTE   0xFD

typedef struct {
    const char* file;
    size_t      live_bytes;
    size_t      total_bytes;
    size_t      alloc_count;
} KfxSite;

typedef struct {
    void*       ptr;        /* pointer returned to caller (past prefix) */
    size_t      size;       /* user-requested size */
    const char* file;
    int         line;
} KfxAllocRecord;

static KfxSite        s_sites[KFX_MAX_SITES];
static int            s_nsites         = 0;
static size_t         s_total_live     = 0;
static KfxAllocRecord s_allocs[KFX_MAX_ALLOCS];
static int            s_nallocs        = 0;
static int            s_sites_overflow = 0;

static KfxSite* get_site(const char* file)
{
    int i;
    for (i = 0; i < s_nsites; i++)
        if (s_sites[i].file == file) return &s_sites[i];
    if (s_nsites < KFX_MAX_SITES) {
        s_sites[s_nsites].file = file;
        return &s_sites[s_nsites++];
    }
    if (!s_sites_overflow) {
        fprintf(stderr, "WARNING: KfxMemDump exceeded KFX_MAX_SITES (%d); tracking disabled for new sites\n", KFX_MAX_SITES);
        s_sites_overflow = 1;
    }
    return NULL;
}

static void record(size_t size, const char* file, int line)
{
    KfxSite* s;
    s = file ? get_site(file) : NULL;
    if (s) { s->live_bytes += size; s->total_bytes += size; s->alloc_count++; }
    s_total_live += size;
    (void)line;
}

static void record_ptr(void* ptr, size_t size, const char* file, int line)
{
    if (s_nallocs < KFX_MAX_ALLOCS) {
        s_allocs[s_nallocs].ptr  = ptr;
        s_allocs[s_nallocs].size = size;
        s_allocs[s_nallocs].file = file;
        s_allocs[s_nallocs].line = line;
        s_nallocs++;
    }
}

static size_t find_and_remove_ptr(void* ptr)
{
    int i;
    for (i = 0; i < s_nallocs; i++) {
        if (s_allocs[i].ptr == ptr) {
            size_t size = s_allocs[i].size;
            s_allocs[i] = s_allocs[s_nallocs - 1];
            s_nallocs--;
            return size;
        }
    }
    return 0;
}

/* --- Guard-zone helpers --- */

/* Write prefix (size_t + pad to KFX_GUARD_SIZE) and suffix canaries.
 * Returns the user-visible pointer (past the prefix). */
static void* guard_wrap(void* raw, size_t user_size)
{
    unsigned char* base = (unsigned char*)raw;
    /* prefix: store size, then fill remaining prefix bytes with canary */
    memcpy(base, &user_size, sizeof(size_t));
    memset(base + sizeof(size_t), KFX_GUARD_BYTE, KFX_GUARD_SIZE - sizeof(size_t));
    /* suffix canary */
    memset(base + KFX_GUARD_SIZE + user_size, KFX_GUARD_BYTE, KFX_GUARD_SIZE);
    return base + KFX_GUARD_SIZE;
}

/* Validate canaries around a user pointer.  Returns 1 if OK, 0 if corrupt. */
static int guard_check(void* user_ptr, size_t user_size, const char* context)
{
    unsigned char* base = (unsigned char*)user_ptr - KFX_GUARD_SIZE;
    int ok = 1;
    size_t i;

    /* check prefix canary bytes (after the stored size_t) */
    for (i = sizeof(size_t); i < KFX_GUARD_SIZE; i++) {
        if (base[i] != KFX_GUARD_BYTE) { ok = 0; break; }
    }
    if (!ok) {
        fprintf(stderr, "KFX GUARD CORRUPTION [%s]: PREFIX overwrite detected at %p (size=%lu)\n",
                context, user_ptr, (unsigned long)user_size);
        fflush(stderr);
        abort();
    }

    /* check suffix canary */
    {
        unsigned char* suffix = (unsigned char*)user_ptr + user_size;
        for (i = 0; i < KFX_GUARD_SIZE; i++) {
            if (suffix[i] != KFX_GUARD_BYTE) { ok = 0; break; }
        }
    }
    if (!ok) {
        fprintf(stderr, "KFX GUARD CORRUPTION [%s]: SUFFIX overwrite detected at %p (size=%lu)\n",
                context, user_ptr, (unsigned long)user_size);
        fflush(stderr);
        abort();
    }
    return ok;
}

/* Recover the raw malloc pointer from a user pointer */
static void* guard_raw(void* user_ptr)
{
    return (unsigned char*)user_ptr - KFX_GUARD_SIZE;
}

void* KfxAlloc_impl(size_t size, const char* file, int line)
{
    void* raw = malloc(size + KFX_GUARD_SIZE * 2);
    if (!raw && size) kfx_oom(size, file, line);
    void* user = guard_wrap(raw, size);
    record(size, file, line);
    record_ptr(user, size, file, line);
    return user;
}

void* KfxCalloc_impl(size_t count, size_t size, const char* file, int line)
{
    size_t total = count * size;
    void* raw = malloc(total + KFX_GUARD_SIZE * 2);
    if (!raw && count && size) kfx_oom(total, file, line);
    if (raw) memset(raw, 0, total + KFX_GUARD_SIZE * 2);
    void* user = guard_wrap(raw, total);
    /* calloc: re-zero the user region (guard_wrap wrote canaries into prefix/suffix) */
    memset(user, 0, total);
    record(total, file, line);
    record_ptr(user, total, file, line);
    return user;
}

void* KfxRealloc_impl(void* ptr, size_t size, const char* file, int line)
{
    size_t old_size;
    if (size == 0) {
        if (ptr) {
            old_size = find_and_remove_ptr(ptr);
            if (old_size) {
                guard_check(ptr, old_size, "realloc(free)");
                s_total_live -= old_size;
            }
            free(guard_raw(ptr));
        }
        return NULL;
    }
    if (!ptr)
        return KfxAlloc_impl(size, file, line);

    old_size = find_and_remove_ptr(ptr);
    if (old_size) {
        guard_check(ptr, old_size, "realloc");
        s_total_live -= old_size;
    }

    void* raw = realloc(guard_raw(ptr), size + KFX_GUARD_SIZE * 2);
    if (!raw) kfx_oom(size, file, line);
    void* user = guard_wrap(raw, size);
    record(size, file, line);
    record_ptr(user, size, file, line);
    return user;
}

void KfxFree(void* ptr)
{
    size_t size;
    if (!ptr) return;
    size = find_and_remove_ptr(ptr);
    if (size) {
        guard_check(ptr, size, "free");
        s_total_live -= size;
        /* Poison freed memory to make use-after-free more obvious */
        memset(ptr, 0xDD, size);
        free(guard_raw(ptr));
    } else {
        /* Allocation not tracked (overflow of s_allocs table) — free directly.
         * Read the size from the prefix to still validate canaries. */
        unsigned char* base = (unsigned char*)ptr - KFX_GUARD_SIZE;
        memcpy(&size, base, sizeof(size_t));
        guard_check(ptr, size, "free(untracked)");
        free(base);
    }
}

char* KfxStrDup_impl(const char* s, const char* file, int line)
{
    size_t len;
    if (!s) return NULL;
    len = strlen(s) + 1;
    char* p = (char*)KfxAlloc_impl(len, file, line);
    memcpy(p, s, len);
    return p;
}

void KfxMemDump(void)
{
    int i;
    fprintf(stderr, "=== KfxMemDump: %lu bytes live (%d tracked allocs) ===\n",
            (unsigned long)s_total_live, s_nallocs);
    for (i = 0; i < s_nsites; i++) {
        const char* f = s_sites[i].file;
        const char* sl = strrchr(f, '/');
        if (!sl) sl = strrchr(f, '\\');
        fprintf(stderr, "  %-45s  live=%9lu  allocs=%lu\n",
                sl ? sl + 1 : f,
                (unsigned long)s_sites[i].live_bytes,
                (unsigned long)s_sites[i].alloc_count);
    }
}

void KfxMemValidate(void)
{
    int i;
    int errors = 0;
    for (i = 0; i < s_nallocs; i++) {
        unsigned char* base = (unsigned char*)s_allocs[i].ptr - KFX_GUARD_SIZE;
        size_t user_size = s_allocs[i].size;
        size_t j;
        int prefix_ok = 1, suffix_ok = 1;

        for (j = sizeof(size_t); j < KFX_GUARD_SIZE; j++) {
            if (base[j] != KFX_GUARD_BYTE) { prefix_ok = 0; break; }
        }
        {
            unsigned char* suffix = (unsigned char*)s_allocs[i].ptr + user_size;
            for (j = 0; j < KFX_GUARD_SIZE; j++) {
                if (suffix[j] != KFX_GUARD_BYTE) { suffix_ok = 0; break; }
            }
        }
        if (!prefix_ok || !suffix_ok) {
            fprintf(stderr, "KFX GUARD CORRUPTION [validate]: alloc %p size=%lu from %s:%d (%s%s)\n",
                    s_allocs[i].ptr, (unsigned long)user_size,
                    s_allocs[i].file ? s_allocs[i].file : "?",
                    s_allocs[i].line,
                    prefix_ok ? "" : "PREFIX ",
                    suffix_ok ? "" : "SUFFIX");
            errors++;
        }
    }
    if (errors) {
        fprintf(stderr, "KfxMemValidate: %d CORRUPTED allocation(s) found — aborting\n", errors);
        fflush(stderr);
        abort();
    }
    fprintf(stderr, "KfxMemValidate: %d allocations OK (%lu bytes live)\n",
            s_nallocs, (unsigned long)s_total_live);
}

#endif /* KFX_DEBUG_MEMORY */

/* ===================================================================
 * Scratch / arena allocator (shared release + debug)
 * =================================================================== */
static unsigned char* s_scratch_base = NULL;
static size_t         s_scratch_cap  = 0;
static size_t         s_scratch_used = 0;

void KfxMemInit(void)
{
    // size_t cap = PlatformManager_GetScratchSize();
    size_t cap = 4 * 1024 * 1024; /* default to 16MB if platform manager not available */
    s_scratch_base = (unsigned char*)malloc(cap);
    if (!s_scratch_base) kfx_oom(cap, __FILE__, __LINE__);
    s_scratch_cap  = cap;
    s_scratch_used = 0;
}

void KfxMemShutdown(void)
{
    free(s_scratch_base);
    s_scratch_base = NULL;
    s_scratch_cap  = 0;
    s_scratch_used = 0;
}

void* KfxScratch(size_t size)
{
    void* p;
    /* align to 8 bytes */
    size = (size + 7u) & ~7u;
    if (s_scratch_base && s_scratch_used + size <= s_scratch_cap) {
        p = s_scratch_base + s_scratch_used;
        s_scratch_used += size;
        return p;
    }
    /* Overflow: fall back to malloc as emergency allocation.
     * WARNING: caller must NOT call KfxFree on overflow allocations;
     * they are tracked separately and freed only at shutdown. */
    fprintf(stderr, "WARNING: KfxScratch overflow (used=%lu cap=%lu req=%lu); using heap\n",
            (unsigned long)s_scratch_used, (unsigned long)s_scratch_cap, (unsigned long)size);
    return malloc(size);
}

void KfxScratchReset(void)
{
    s_scratch_used = 0;
}

size_t KfxScratchUsed(void)
{
    return s_scratch_used;
}
