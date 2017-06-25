#ifndef CTEST__TESTS__ASSERT_H__INCLUDED__
#define CTEST__TESTS__ASSERT_H__INCLUDED__

#include <inttypes.h>
#include <string.h>

#include <ctest/_annotations.h>
#include <ctest/_preprocessor.h>

#define CT_FAIL(...)                            CTEST_FAIL__("" __VA_ARGS__)

#define CT_ASSERT(expr, ...)                    CT_ASSERT__(expr, "" __VA_ARGS__)

#define CT_ASSERT_UINT_EQ(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_EQ__, CTEST_OPERATOR_EQ_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_UINT_NE(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_NE__, CTEST_OPERATOR_NE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_UINT_LT(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_LT__, CTEST_OPERATOR_LT_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_UINT_LE(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_LE__, CTEST_OPERATOR_LE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_UINT_GT(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_GT__, CTEST_OPERATOR_GT_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_UINT_GE(act, exp, ...)        CTEST_ASSERT_CMP__(uintmax_t, act, exp, CTEST_OPERATOR_GE__, CTEST_OPERATOR_GE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)

#define CT_ASSERT_INT_EQ(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_EQ__, CTEST_OPERATOR_EQ_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_INT_NE(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_NE__, CTEST_OPERATOR_NE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_INT_LT(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_LT__, CTEST_OPERATOR_LT_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_INT_LE(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_LE__, CTEST_OPERATOR_LE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_INT_GT(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_GT__, CTEST_OPERATOR_GT_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_INT_GE(act, exp, ...)         CTEST_ASSERT_CMP__(intmax_t, act, exp, CTEST_OPERATOR_GE__, CTEST_OPERATOR_GE_STR__, "%ju", CTEST_FMTR_NOOP__, "" __VA_ARGS__)

#define CT_ASSERT_STR_EQ(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STREQ__, CTEST_OPERATOR_STREQ_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_STR_NE(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STRNE__, CTEST_OPERATOR_STRNE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_STR_LT(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STRLT__, CTEST_OPERATOR_STRLT_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_STR_LE(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STRLE__, CTEST_OPERATOR_STRLE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_STR_GT(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STRGT__, CTEST_OPERATOR_STRGT_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_STR_GE(act, exp, ...)         CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_STRGE__, CTEST_OPERATOR_STRGE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)

#define CT_ASSERT_ISTR_EQ(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTREQ__, CTEST_OPERATOR_ISTREQ_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_ISTR_NE(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTRNE__, CTEST_OPERATOR_ISTRNE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_ISTR_LT(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTRLT__, CTEST_OPERATOR_ISTRLT_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_ISTR_LE(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTRLE__, CTEST_OPERATOR_ISTRLE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_ISTR_GT(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTRGT__, CTEST_OPERATOR_ISTRGT_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_ISTR_GE(act, exp, ...)        CTEST_ASSERT_CMP__(const char *, act, exp, CTEST_OPERATOR_ISTRGE__, CTEST_OPERATOR_ISTRGE_STR__, "\"%s\"", CTEST_FMTR_NOOP__, "" __VA_ARGS__)

#define CT_ASSERT_PTR_EQ(act, exp, ...)         CTEST_ASSERT_CMP__(void *, act, exp, CTEST_OPERATOR_EQ__, CTEST_OPERATOR_EQ_STR__, "%p", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_PTR_NE(act, exp, ...)         CTEST_ASSERT_CMP__(void *, act, exp, CTEST_OPERATOR_NE__, CTEST_OPERATOR_NE_STR__, "%p", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_NULL(act, ...)                CTEST_ASSERT_CMP__(void *, act, NULL, CTEST_OPERATOR_EQ__, CTEST_OPERATOR_EQ_STR__, "%p", CTEST_FMTR_NOOP__, "" __VA_ARGS__)
#define CT_ASSERT_NONNULL(act, ...)             CTEST_ASSERT_CMP__(void *, act, NULL, CTEST_OPERATOR_EQ__, CTEST_OPERATOR_NE_STR__, "%p", CTEST_FMTR_NOOP__, "" __VA_ARGS__)

#define CT_ASSERT_BOOL_EQ(act, exp, ...)        CTEST_ASSERT_CMP__(int, act, exp, CTEST_OPERATOR_BOOLEQ__, CTEST_OPERATOR_BOOLEQ_STR__, "%s", CTEST_FMTR_BOOL__, "" __VA_ARGS__)
#define CT_ASSERT_BOOL_NE(act, exp, ...)        CTEST_ASSERT_CMP__(int, act, exp, CTEST_OPERATOR_BOOLNE__, CTEST_OPERATOR_BOOLNE_STR__, "%s", CTEST_FMTR_BOOL__, "" __VA_ARGS__)
#define CT_ASSERT_TRUE(act, ...)                CTEST_ASSERT_CMP__(int, act, true, CTEST_OPERATOR_BOOLEQ__, CTEST_OPERATOR_BOOLEQ_STR__, "%s", CTEST_FMTR_BOOL__, "" __VA_ARGS__)
#define CT_ASSERT_FALSE(act, ...)               CTEST_ASSERT_CMP__(int, act, false, CTEST_OPERATOR_BOOLEQ__, CTEST_OPERATOR_BOOLNE_STR__, "%s", CTEST_FMTR_BOOL__, "" __VA_ARGS__)

#define CT_ASSERT__(expr, fmt, ...)             CTEST_ASSERT__(expr, "%s failed" fmt,  CTEST_STRINGIZE__(expr), ##__VA_ARGS__)

#define CTEST_FAIL__(fmt, ...) \
	do { \
		ctest_fail(__FILE__, __LINE__, fmt,  ##__VA_ARGS__); \
	} while(0)

#define CTEST_ASSERT__(expr, fmt, ...) \
	do { \
		if (!(expr)) CT_FAIL(fmt, ## __VA_ARGS__); \
	} while(0)

#define CTEST_ASSERT_CMP__(type, actual, expect, operator_, operator_str, operand_fmt, operand_fmtr, fmt, ...) \
	do { \
		type const actual__ = actual; \
		type const expect__ = expect; \
		if (strlen(fmt) > 0) { \
			CTEST_ASSERT__( \
				operator_(actual__, expect__), \
				"%s evaluated to " operand_fmt " but should " operator_str operand_fmt ": " fmt, \
				#actual, operand_fmtr(actual__), operand_fmtr(expect__), ##__VA_ARGS__); \
		} else { \
			CTEST_ASSERT__( \
				operator_(actual__, expect__), \
				"%s evaluated to " operand_fmt " but should " operator_str operand_fmt "", \
				#actual, operand_fmtr(actual__), operand_fmtr(expect__)); \
		} \
	} while(0)

#define CTEST_FMTR_NOOP__(x)            (x)
#define CTEST_FMTR_BOOL__(x)            ((x) ? "true" : "false")

#define CTEST_OPERATOR_EQ__(a, b)       ((a) == (b))
#define CTEST_OPERATOR_EQ_STR__         "be "
#define CTEST_OPERATOR_NE__(a, b)       ((a) != (b))
#define CTEST_OPERATOR_NE_STR__         "be different from "
#define CTEST_OPERATOR_LT__(a, b)       ((a) <  (b))
#define CTEST_OPERATOR_LT_STR__         "be less than "
#define CTEST_OPERATOR_LE__(a, b)       ((a) <= (b))
#define CTEST_OPERATOR_LE_STR__         "be no greater than "
#define CTEST_OPERATOR_GT__(a, b)       ((a) >  (b))
#define CTEST_OPERATOR_GT_STR__         "be greater than "
#define CTEST_OPERATOR_GE__(a, b)       ((a) >= (b))
#define CTEST_OPERATOR_GE_STR__         "be no less than "

#define CTEST_OPERATOR_STREQ__(a, b)    (strcmp(a, b) == 0)
#define CTEST_OPERATOR_STREQ_STR__      CTEST_OPERATOR_EQ_STR__
#define CTEST_OPERATOR_STRNE__(a, b)    (strcmp(a, b) != 0)
#define CTEST_OPERATOR_STRNE_STR__      CTEST_OPERATOR_NE_STR__
#define CTEST_OPERATOR_STRLT__(a, b)    (strcmp(a, b) <  0)
#define CTEST_OPERATOR_STRLT_STR__      "lexicographically preceded "
#define CTEST_OPERATOR_STRLE__(a, b)    (strcmp(a, b) <= 0)
#define CTEST_OPERATOR_STRLE_STR__      "not lexicographically follow "
#define CTEST_OPERATOR_STRGT__(a, b)    (strcmp(a, b) >  0)
#define CTEST_OPERATOR_STRGT_STR__      "lexicographically follow "
#define CTEST_OPERATOR_STRGE__(a, b)    (strcmp(a, b) >= 0)
#define CTEST_OPERATOR_STRGE_STR__      "not lexicographically precede "

#define CTEST_OPERATOR_STR_IGNORECASE__ "(ignoring case) "
#define CTEST_OPERATOR_ISTREQ__(a, b)   (strcasecmp(a, b) == 0)
#define CTEST_OPERATOR_ISTREQ_STR__     CTEST_OPERATOR_STREQ_STR__ CTEST_OPERATOR_STR_IGNORECASE__
#define CTEST_OPERATOR_ISTRNE__(a, b)   (strcasecmp(a, b) != 0)
#define CTEST_OPERATOR_ISTRNE_STR__     CTEST_OPERATOR_STRNE_STR__ CTEST_OPERATOR_STR_IGNORECASE__
#define CTEST_OPERATOR_ISTRLT__(a, b)   (strcasecmp(a, b) <  0)
#define CTEST_OPERATOR_ISTRLT_STR__     CTEST_OPERATOR_STRLT_STR__ CTEST_OPERATOR_STR_IGNORECASE__
#define CTEST_OPERATOR_ISTRLE__(a, b)   (strcasecmp(a, b) <= 0)
#define CTEST_OPERATOR_ISTRLE_STR__     CTEST_OPERATOR_STRLE_STR__ CTEST_OPERATOR_STR_IGNORECASE__
#define CTEST_OPERATOR_ISTRGT__(a, b)   (strcasecmp(a, b) >  0)
#define CTEST_OPERATOR_ISTRGT_STR__     CTEST_OPERATOR_STRGT_STR__ CTEST_OPERATOR_STR_IGNORECASE__
#define CTEST_OPERATOR_ISTRGE__(a, b)   (strcasecmp(a, b) >= 0)
#define CTEST_OPERATOR_ISTRGE_STR__     CTEST_OPERATOR_STRGE_STR__ CTEST_OPERATOR_STR_IGNORECASE__

#define CTEST_OPERATOR_BOOLEQ__(a, b)   (!(a) == !(b))
#define CTEST_OPERATOR_BOOLEQ_STR__     CTEST_OPERATOR_EQ_STR__
#define CTEST_OPERATOR_BOOLNE__(a, b)   (!(a) != !(b))
#define CTEST_OPERATOR_BOOLNE_STR__     CTEST_OPERATOR_EQ_STR__

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct ctest_context ctest_context_t;
struct ctest_context {
	const char *file;
	int line;
};

CTEST_PRINTF__(3, 4) CTEST_NORETURN__
extern void ctest_fail(const char *file, int line, const char *fmt, ...);

CTEST_PRINTF__(3, 4) CTEST_NORETURN__
extern void ctest_skip(const char *file, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* CTEST__TESTS__ASSERT_H__INCLUDED__ */

