#ifndef PRIVATE__UTILS_H__INCLUDED__
#define PRIVATE__UTILS_H__INCLUDED__

#include <stddef.h>
#include <ctest/_preprocessor.h>

#ifndef offsetof
#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#endif

#define containerof(ptr, type, member) ({ \
                 const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                 (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * Determine the alignment of a type of object.
 */
#define alignmentof(type)       ((size_t)&(((struct { char ch; type t; } *)0)->t))

#ifdef __GNUC__
#define unused(arg)	CTEST_GLUE__(unused_arg__, arg) __attribute__((unused))
#else
#define unused(arg)	CTEST_GLUE__(unused_arg__, arg)
#endif

#define countof(array)  (sizeof(array)/sizeof(*(array)))

#endif /* PRIVATE__UTILS_H__INCLUDED__ */
