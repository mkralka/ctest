/**
 * Function annotation hints for the compiler.
 */

#ifndef CTEST__ANNOTATIONS_H__INCLUDED__
#define CTEST__ANNOTATIONS_H__INCLUDED__

#ifdef __GNUC__
#define CTEST_ALL_NONNULL_ARGS__        __attribute__((nonnull))
#define CTEST_NONNULL_ARGS__(...)       __attribute__((nonnull(__VA_ARGS__)))
#define CTEST_NORETURN__                __attribute__((noreturn))
#define CTEST_PRINTF__(fmt,opt)         __attribute__((format(printf,fmt,opt)))
#define CTEST_VPRINTF__(fmt)            __attribute__((format(printf,fmt,0)))
#else
#define CTEST_ALL_NONNULL_ARGS__
#define CTEST_NONNULL_ARGS__(...)
#define CTEST_NORETURN__
#define CTEST_PRINTF__(fmt,opt)
#define CTEST_VPRINTF__(fmt)
#endif

#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 409
#define CTEST_RETURNS_NONNULL__         __attribute__((returns_nonnull))
#else
#define CTEST_RETURNS_NONNULL__
#endif

#endif /* CTEST__ANNOTATIONS_H__INCLUDED__ */
