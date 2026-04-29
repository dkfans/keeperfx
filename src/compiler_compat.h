/**
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * compiler_compat.h — GCC to MSVC attribute compatibility
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *
 * Maps GCC-specific function attributes (__attribute__) to MSVC/Clang equivalents
 * or provides no-op fallbacks. Allows single-source compatibility across compilers.
 *
 * Include this header at the top of any .h/.hpp file using __attribute__.
 */

#ifndef COMPILER_COMPAT_H
#define COMPILER_COMPAT_H

// ━━━ Compiler Detection ━━━
#if defined(_MSC_VER)
    // MSVC (cl.exe)
    #define KFX_COMPILER_MSVC 1
    // MSVC does not support __attribute__ at all — define it away so existing
    // code using raw __attribute__((nonnull)), __attribute__((format)) etc. compiles.
    #ifndef __attribute__
        #define __attribute__(x)
    #endif
    // __builtin_offsetof is a GCC extension; map it to standard offsetof().
    // offsetof() is a compile-time constant on MSVC (unlike address arithmetic).
    #include <stddef.h>
    #ifndef __builtin_offsetof
        #define __builtin_offsetof(type, member) offsetof(type, member)
    #endif
#elif defined(__GNUC__)
    // GCC or Clang (defines __GNUC__ for compatibility)
    #define KFX_COMPILER_GCC 1
#else
    #define KFX_COMPILER_UNKNOWN 1
#endif

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// GCC Attribute Wrappers
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * KFX_NONNULL(indices...)
 * Marks specific parameters as never NULL. GCC warns at compile-time if NULL is passed.
 * MSVC doesn't have direct equivalent, but can use _Pragma disable for SAL warnings.
 *
 * Usage:
 *   int str_len(const char *str) KFX_NONNULL(1);
 *   int str_compare(const char *a, const char *b) KFX_NONNULL(1, 2);
 */
#if defined(KFX_COMPILER_GCC)
    #define KFX_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#elif defined(KFX_COMPILER_MSVC)
    // MSVC: suppress /analyze warnings about potential NULL parameters if desired
    // Most code doesn't use /analyze, so nonnull is a no-op here
    #define KFX_NONNULL(...)
#else
    #define KFX_NONNULL(...)
#endif

/**
 * KFX_PRINTF_FORMAT(fmt_idx, va_idx)
 * Validates printf-style format string at index fmt_idx, with varargs starting at va_idx.
 * Enables -Wformat warnings in GCC/Clang; MSVC has no direct equivalent.
 *
 * Usage:
 *   int my_printf(const char *fmt, ...) KFX_PRINTF_FORMAT(1, 2);
 *   int my_snprintf(char *buf, int size, const char *fmt, ...) KFX_PRINTF_FORMAT(3, 4);
 */
#if defined(KFX_COMPILER_GCC)
    #define KFX_PRINTF_FORMAT(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
#elif defined(KFX_COMPILER_MSVC)
    // MSVC: no direct equivalent. SAL annotations (e.g., _Printf_format_string_) exist
    // but require Windows SDK headers and are less portable. Skip for now.
    #define KFX_PRINTF_FORMAT(fmt_idx, va_idx)
#else
    #define KFX_PRINTF_FORMAT(fmt_idx, va_idx)
#endif

/**
 * KFX_UNUSED
 * Marks a variable/parameter as intentionally unused. Suppresses -Wunused warnings.
 * C++17 standardizes this as [[maybe_unused]]; we provide backward compat.
 *
 * Usage:
 *   void handler(int event KFX_UNUSED) { }
 *   KFX_UNUSED int debug_val = some_expensive_computation();
 */
#if __cplusplus >= 201703L || __STDC_VERSION__ >= 202303L
    // C++17 or C23+: use standard attribute
    #define KFX_UNUSED [[maybe_unused]]
#elif defined(KFX_COMPILER_GCC)
    #define KFX_UNUSED __attribute__((unused))
#elif defined(KFX_COMPILER_MSVC)
    // MSVC: __pragma to suppress C4100 (unreferenced formal parameter)
    // Or just leave blank; MSVC warnings for unused params are less noisy than GCC.
    #define KFX_UNUSED
#else
    #define KFX_UNUSED
#endif

/**
 * KFX_DEPRECATED
 * Marks a function/variable as deprecated. Emits warning if used.
 * Useful for gradual API migration.
 *
 * Usage:
 *   KFX_DEPRECATED int old_api() { return 42; }
 */
#if defined(KFX_COMPILER_GCC)
    #define KFX_DEPRECATED __attribute__((deprecated))
#elif defined(KFX_COMPILER_MSVC)
    #define KFX_DEPRECATED __declspec(deprecated)
#else
    #define KFX_DEPRECATED
#endif

/**
 * KFX_LIKELY / KFX_UNLIKELY
 * Branch prediction hints for performance-critical paths.
 * GCC/Clang: __builtin_expect. MSVC: __assume or no-op (optimizer is smart enough).
 *
 * Usage:
 *   if (KFX_LIKELY(x > 0)) { }
 *   if (KFX_UNLIKELY(err)) { }
 */
#if defined(KFX_COMPILER_GCC)
    #define KFX_LIKELY(x) __builtin_expect(!!(x), 1)
    #define KFX_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    // MSVC/others: no-op; modern optimizers don't need hints
    #define KFX_LIKELY(x) (x)
    #define KFX_UNLIKELY(x) (x)
#endif

/**
 * KFX_INLINE / KFX_FORCE_INLINE
 * Inlining hints. GCC/Clang: inline/always_inline. MSVC: inline/__forceinline.
 *
 * Usage:
 *   KFX_INLINE int min(int a, int b) { return a < b ? a : b; }
 *   KFX_FORCE_INLINE int fast_path() { }
 */
#if defined(KFX_COMPILER_MSVC)
    #define KFX_INLINE inline
    #define KFX_FORCE_INLINE __forceinline
#elif defined(KFX_COMPILER_GCC)
    #define KFX_INLINE inline
    #define KFX_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define KFX_INLINE inline
    #define KFX_FORCE_INLINE inline
#endif

/**
 * KFX_NORETURN
 * Marks a function that never returns (e.g., exit, abort, infinite loop).
 * Helps optimizer and static analysis tools.
 *
 * Usage:
 *   KFX_NORETURN void fatal_error(const char *msg);
 */
#if __cplusplus >= 201103L || __STDC_VERSION__ >= 201112L
    // C++11 or C11+: use standard _Noreturn (deprecated in C23) or [[noreturn]]
    #if __cplusplus >= 201703L
        #define KFX_NORETURN [[noreturn]]
    #else
        #define KFX_NORETURN _Noreturn
    #endif
#elif defined(KFX_COMPILER_GCC)
    #define KFX_NORETURN __attribute__((noreturn))
#elif defined(KFX_COMPILER_MSVC)
    #define KFX_NORETURN __declspec(noreturn)
#else
    #define KFX_NORETURN
#endif

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Common Attribute Combinations
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

/**
 * KFX_PRINTF
 * Combines KFX_NONNULL + KFX_PRINTF_FORMAT for printf-like functions.
 * Parameters are 1-indexed (1-based indexing as per GCC convention).
 */
#define KFX_PRINTF(fmt_idx, va_idx) KFX_NONNULL(fmt_idx) KFX_PRINTF_FORMAT(fmt_idx, va_idx)

/**
 * KFX_PURE
 * Function has no side effects and returns same value for same inputs.
 * Allows compiler to optimize aggressive caching.
 *
 * Usage:
 *   KFX_PURE int absolute_value(int x);
 */
#if defined(KFX_COMPILER_GCC)
    #define KFX_PURE __attribute__((pure))
#elif defined(KFX_COMPILER_MSVC)
    // MSVC: no direct equivalent, but intrinsic functions behave this way
    #define KFX_PURE
#else
    #define KFX_PURE
#endif

// ══════════════════════════════════════════════════════════════════════════════
// POSIX Function Compatibility for MSVC
// ══════════════════════════════════════════════════════════════════════════════
#ifdef KFX_COMPILER_MSVC
    #include <direct.h>  // _getcwd, _mkdir
    #include <io.h>      // _access
    
    // MSVC uses underscore-prefixed versions of POSIX functions
    #define access _access
    #define getcwd _getcwd
    #define mkdir(path, mode) _mkdir(path)  // MSVC mkdir takes 1 arg
    
    // File access mode constants
    #ifndef F_OK
        #define F_OK 0  // Test for file existence
    #endif
#endif

#endif // COMPILER_COMPAT_H
