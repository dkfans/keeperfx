/******************************************************************************/
/** @file kfx_memory.h
 *  KeeperFX centralised memory allocator.
 *
 *  Release builds: thin wrappers over malloc/free with OOM abort.
 *  Debug builds (KFX_DEBUG_MEMORY defined): file/line injected via macros,
 *  per-callsite accounting table, KfxMemDump() logs totals.
 *
 *  Scratch allocator: bump-arena for temporary allocations that are freed
 *  in bulk.  Call KfxScratchReset() at end of frame or level-load.
 *  KfxMemInit() must be called once at startup before any KfxScratch call.
 */
/******************************************************************************/
#ifndef KFX_MEMORY_H
#define KFX_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- Lifecycle ---------- */
/** Call once at startup after PlatformManager is registered. */
void   KfxMemInit(void);
void   KfxMemShutdown(void);

/* ---------- Core allocator ---------- */
void*  KfxAlloc(size_t size);
void*  KfxCalloc(size_t count, size_t size);
void*  KfxRealloc(void* ptr, size_t size);
void   KfxFree(void* ptr);
char*  KfxStrDup(const char* s);

/* ---------- Scratch / arena allocator ---------- */
void*  KfxScratch(size_t size);
void   KfxScratchReset(void);
size_t KfxScratchUsed(void);

/* ---------- Diagnostics ---------- */
/** Log per-callsite allocation totals via stderr.  No-op in release builds. */
void   KfxMemDump(void);
/** Walk all tracked allocations and validate guard-zone canaries.
 *  Aborts on corruption.  No-op in release builds. */
void   KfxMemValidate(void);

/* ---------- Debug-mode overrides ---------- */
#ifdef KFX_DEBUG_MEMORY
void*  KfxAlloc_impl(size_t size, const char* file, int line);
void*  KfxCalloc_impl(size_t count, size_t size, const char* file, int line);
void*  KfxRealloc_impl(void* ptr, size_t size, const char* file, int line);
char*  KfxStrDup_impl(const char* s, const char* file, int line);
#define KfxAlloc(sz)        KfxAlloc_impl((sz),       __FILE__, __LINE__)
#define KfxCalloc(n,sz)     KfxCalloc_impl((n),(sz),  __FILE__, __LINE__)
#define KfxRealloc(p,sz)    KfxRealloc_impl((p),(sz), __FILE__, __LINE__)
#define KfxStrDup(s)        KfxStrDup_impl((s),       __FILE__, __LINE__)
#endif /* KFX_DEBUG_MEMORY */

#ifdef __cplusplus
}
#endif

#endif /* KFX_MEMORY_H */
