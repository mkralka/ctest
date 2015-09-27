/*
 * Preprocessor Magic
 */

#ifndef CTEST__PREPROCESSOR_H__INCLUDED__
#define CTEST__PREPROCESSOR_H__INCLUDED__

/* GLUE two or more symbol fragments into a single symbol. */
#define CTEST_GLUE_X__(_1,_2)           _1 ## _2
#define CTEST_GLUE__(_1,_2)             CTEST_GLUE_X__(_1, _2)
#define CTEST_GLUE3__(_1,_2,_3)         CTEST_GLUE__(_1, CTEST_GLUE_X__(_2, _3))
#define CTEST_GLUE4__(_1,_2,_3,_4)      CTEST_GLUE__(CTEST_GLUE_X__(_1, _2), CTEST_GLUE_X__(_3, _4))
#define CTEST_GLUE5__(_1,_2,_3,_4,_5)   CTEST_GLUE3__(CTEST_GLUE_X__(_1, _2), CTEST_GLUE_X__(_3, _4), _5)

#define CTEST_STRINGIZE____(x)          #x
#define CTEST_STRINGIZE___(x)           CTEST_STRINGIZE____(x)
#define CTEST_STRINGIZE__(x)            CTEST_STRINGIZE___(x)

#endif /* CTEST__PREPROCESSOR_H__INCLUDED__ */
