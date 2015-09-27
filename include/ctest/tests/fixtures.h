#ifndef CTEST__TESTS__FIXTURE_H__INCLUDED__
#define CTEST__TESTS__FIXTURE_H__INCLUDED__

#include <ctest/tests/tests.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CTEST_TEMPDIR_PATTERN__	"/tmp/ctest-XXXXXX"

typedef struct ctest_tmpdir_fixture__ ctest_tmpdir_fixture_t;
typedef struct ctest_tmpdir_fixture__ CTEST_FIXTURE_TYPE_NAME__(ctest_tmpdir);
struct ctest_tmpdir_fixture__ {
	char dirname[sizeof(CTEST_TEMPDIR_PATTERN__)];
};

int ctest_tmpdir_init(ctest_tmpdir_fixture_t *fixture);
int ctest_tmpdir_destroy(ctest_tmpdir_fixture_t *fixture);

extern ctest_def_fixture_provider_t__ CTEST_FIXTURE_NAME__(ctest_tmpdir);

#ifdef __cplusplus
}
#endif


#endif /* CTEST__TESTS__FIXTURE_H__INCLUDED__ */

