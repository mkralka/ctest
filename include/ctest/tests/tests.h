#ifndef CTEST__TESTS__TESTS_H__INCLUDED__
#define CTEST__TESTS__TESTS_H__INCLUDED__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <ctest/_annotations.h>
#include <ctest/_preprocessor.h>

/*
 * Test Data
 */
#define CT_DATA_TYPE(name) \
	struct CTEST_DATA_NAME__(name)
#define CT_DATA(name) \
	typedef const struct CTEST_DATA_NAME__(name) CTEST_DATA_TYPE_NAME__(name); \
	static CTEST_DATA_TYPE_NAME__(name) CTEST_DATA_NAME__(name)[] =
#define CT_DATA_PROVIDER(name, fmt, ...) \
	static int CTEST_DATA_TOSTRING_NAME__(name)(char *buf, size_t buflen, const CTEST_DATA_TYPE_NAME__(name)* data) \
	{ \
		return snprintf(buf, buflen, fmt, ##__VA_ARGS__); \
	} \
	static ctest_def_data_provider_t__ CTEST_DATA_PROVIDER_NAME__(name) = { \
		CTEST_DATA_NAME__(name),\
		sizeof(CTEST_DATA_NAME__(name))/sizeof(CTEST_DATA_NAME__(name)[0]), \
		sizeof(CTEST_DATA_NAME__(name)[0]), \
		(int (*)(char *, size_t, const void *))&CTEST_DATA_TOSTRING_NAME__(name), \
	}

/**
 * Test Fixtures
 */
#define CT_FIXTURE_TYPE(name) \
	struct CTEST_FIXTURE_NAME__(name)
#define CT_FIXTURE_SETUP(name) \
	static void CTEST_FIXTURE_SETUP_NAME__(name)(struct CTEST_FIXTURE_NAME__(name) *fixture)
#define CT_FIXTURE_TEARDOWN(name) \
	static void CTEST_FIXTURE_TEARDOWN_NAME__(name)(struct CTEST_FIXTURE_NAME__(name) *fixture)
#define CT_FIXTURE(name) \
	typedef struct CTEST_FIXTURE_NAME__(name) CTEST_FIXTURE_TYPE_NAME__(name); \
	static ctest_def_fixture_provider_t__ CTEST_FIXTURE_NAME__(name) = { \
		(void(*)(void*))&CTEST_FIXTURE_SETUP_NAME__(name), \
		(void(*)(void*))&CTEST_FIXTURE_TEARDOWN_NAME__(name), \
		sizeof(CTEST_FIXTURE_TYPE_NAME__(name)), \
	}

/*
 * Tests
 */
#define CT_TEST(name) \
	static void CTEST_TEST_NAME__(name)(void); \
	static void CTEST_TEST_CALLER_NAME__(name)(void *vf, const void *vd) \
	{ \
		(void)vf; \
		(void)vd; \
		return CTEST_TEST_NAME__(name)(); \
	} \
	CTEST_TEST_DEF__(name, NULL, NULL); \
	static void CTEST_TEST_NAME__(name)(void)

#define CT_TEST_WITH_DATA(name, data_name) \
	static void CTEST_TEST_NAME__(name)(CTEST_DATA_TYPE_NAME__(data_name) *); \
	static void CTEST_TEST_CALLER_NAME__(name)(void *vf, const void *vd) \
	{ \
		CTEST_DATA_TYPE_NAME__(data_name)*const d = (void*)vd; \
		(void)vf; \
		return CTEST_TEST_NAME__(name)(d); \
	} \
	CTEST_TEST_DEF__(name, NULL, &CTEST_DATA_PROVIDER_NAME__(data_name)); \
	static void CTEST_TEST_NAME__(name)(CTEST_DATA_TYPE_NAME__(data_name) *data)

#define CT_TEST_WITH_FIXTURE(name, fixture_name) \
	static void CTEST_TEST_NAME__(name)(CTEST_FIXTURE_TYPE_NAME__(fixture_name) *); \
	static void CTEST_TEST_CALLER_NAME__(name)(void *vf, const void *vd) \
	{ \
		CTEST_FIXTURE_TYPE_NAME__(fixture_name)*const f = vf; \
		(void)vd; \
		return CTEST_TEST_NAME__(name)(f); \
	} \
	CTEST_TEST_DEF__(name, &CTEST_FIXTURE_NAME__(fixture_name), NULL); \
	static void CTEST_TEST_NAME__(name)(CTEST_FIXTURE_TYPE_NAME__(fixture_name) *fixture)

#define CT_TEST_WITH_FIXTURE_AND_DATA(name, fixture_name, data_name) \
	static void CTEST_TEST_NAME__(name)(CTEST_FIXTURE_TYPE_NAME__(fixture_name) *, CTEST_DATA_TYPE_NAME__(data_name) *); \
	static void CTEST_TEST_CALLER_NAME__(name)(void *vf, const void *vd) \
	{ \
		CTEST_FIXTURE_TYPE_NAME__(fixture_name)*const f = vf; \
		CTEST_DATA_TYPE_NAME__(data_name)*const d = (void*)vd; \
		return CTEST_TEST_NAME__(name)(f, d); \
	} \
	CTEST_TEST_DEF__(name, &CTEST_FIXTURE_NAME__(fixture_name), &CTEST_DATA_PROVIDER_NAME__(data_name)); \
	static void CTEST_TEST_NAME__(name)(CTEST_FIXTURE_TYPE_NAME__(fixture_name) *fixture, CTEST_DATA_TYPE_NAME__(data_name) *data)

/*
 * Test Suite
 */
#define CT_SUITE_TESTS(name) \
	static ctest_def_test_t__ *const CTEST_SUITE_TESTS_NAME__(name)[] =

#define CT_SUITE_TEST(name) \
	&CTEST_TEST_DEF_NAME__(name)

#define CT_SUITE(name) \
	ctest_def_suite_t__ CTEST_SUITE_SYMBOL__ = { \
		CTEST_SUITE_MAGIC__, \
		CTEST_SUITE_VERSION__, \
		#name, \
		CTEST_SUITE_TESTS_NAME__(name), \
		sizeof(CTEST_SUITE_TESTS_NAME__(name))/sizeof(CTEST_SUITE_TESTS_NAME__(name)[0]), \
	}

/*
 * Symbols
 */
#define CTEST_DATA_TYPE_NAME__(name)                    CTEST_GLUE3__(ctest_data__,name,__t__)
#define CTEST_DATA_TOSTRING_NAME__(name)                CTEST_GLUE3__(ctest_data__,name,__to_string__)
#define CTEST_DATA_PROVIDER_NAME__(name)                CTEST_GLUE3__(ctest_data__,name,__provider__)
#define CTEST_DATA_NAME__(name)                         CTEST_GLUE3__(ctest_data__,name,__)
#define CTEST_TEST_DEF_NAME__(name)                     CTEST_GLUE3__(ctest_test__,name,__def__)
#define CTEST_TEST_CALLER_NAME__(name)                  CTEST_GLUE3__(ctest_test__,name,__caller__)
#define CTEST_TEST_NAME__(name)                         CTEST_GLUE3__(ctest_test__,name,__)
#define CTEST_FIXTURE_TYPE_NAME__(name)                 CTEST_GLUE3__(ctest_fixture__,name,__t__)
#define CTEST_FIXTURE_SETUP_NAME__(name)                CTEST_GLUE3__(ctest_fixture__,name,__setup__)
#define CTEST_FIXTURE_TEARDOWN_NAME__(name)             CTEST_GLUE3__(ctest_fixture__,name,__teardown__)
#define CTEST_FIXTURE_NAME__(name)                      CTEST_GLUE3__(ctest_fixture__,name,__)
#define CTEST_SUITE_TESTS_NAME__(name)                  CTEST_GLUE3__(ctest_suite__,name,__tests__)
#define CTEST_SUITE_NAME__(name)                        CTEST_GLUE3__(ctest_suite__,name,__)

/**
 * Define a test definition.
 */
#define CTEST_TEST_DEF__(name, fixture, data) \
	static ctest_def_test_t__ CTEST_TEST_DEF_NAME__(name) = { \
		CTEST_STRINGIZE__(name), \
		&CTEST_TEST_CALLER_NAME__(name), \
		fixture, \
		data, \
	}

#define CTEST_SUITE_SYMBOL__    ctest_suite__
#define CTEST_SUITE_MAGIC__     0x72db2d
#define CTEST_SUITE_VERSION__   0x00000000

typedef const struct ctest_def_fixture_provider__ ctest_def_fixture_provider_t__;
struct ctest_def_fixture_provider__ {
	void (*setup)(void *fixture);
	void (*teardown)(void *fixture);
	size_t size;
};

typedef const struct ctest_def_data_provider__ ctest_def_data_provider_t__;
struct ctest_def_data_provider__ {
	const void *data;
	size_t count;
	size_t size;
	int (*to_string)(char *, size_t, const void *);
};

typedef const struct ctest_def_test__ ctest_def_test_t__;
struct ctest_def_test__ {
	const char *name;
	void (*caller)(void *, const void *);
	ctest_def_fixture_provider_t__ *fixture_provider;
	ctest_def_data_provider_t__ *data_provider;
};

typedef const struct ctest_def_suite__ ctest_def_suite_t__;
struct ctest_def_suite__ {
	uint32_t magic;
	uint32_t version;
	const char *name;
	ctest_def_test_t__ *const *tests;
	size_t test_count;
};

#endif /* CTEST__TESTS__TESTS_H__INCLUDED__ */
