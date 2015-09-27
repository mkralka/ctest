#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctest/tests/assert.h>
#include <ctest/tests/fixtures.h>


static int deltree(const char *dirname)
{
	int result = 0;

	/* Be especially aggressive about deleting the files beneath the
	 * directory by changing the permissions of the directory. This will
	 * cover most cases, except pathological ones where the test manages
	 * to create a directory that we don't own. */
	(void)chmod(dirname, S_IRWXU);

	while (result == 0 && rmdir(dirname) != 0) {
		DIR *dir;
		struct dirent *dirent;

		if (errno != ENOTEMPTY) {
			/* Best effort and we don't deal with this error. */
			result = -1;
			break;
		}

		if ((dir = opendir(dirname)) == NULL) {
			result = -1;
			break;
		}

		errno = 0;
		while ((dirent = readdir(dir)) != NULL) {
			char path[strlen(dirname) + 1 + strlen(dirent->d_name) + 1];

			if (dirent->d_name[0] == '.' && (dirent->d_name[1] == '\0' || (dirent->d_name[1] == '.' && dirent->d_name[2] == '\0')))
				continue;

			snprintf(path, sizeof(path), "%s/%s", dirname, dirent->d_name);
			if (dirent->d_type == DT_DIR) {
				if (deltree(path) != 0)
					result = -1;
			} else {
				if  (unlink(path) != 0)
					result = -1;
			}
		}
		if (errno != 0)
			result = -1;
		closedir(dir);
	}
	return result;
}

static void setup__(void *v)
{
	ctest_tmpdir_fixture_t *const fixture = v;
	if (ctest_tmpdir_init(fixture) != 0) {
		ctest_fail("tmpdir_fixture", __LINE__, "unable to create temporary directory: %s", strerror(errno));
	}
}

static void teardown__(void *v)
{
	ctest_tmpdir_fixture_t *const fixture = v;
	if (ctest_tmpdir_destroy(fixture) != 0) {
		ctest_fail("tmpdir_fixture", __LINE__, "unable to cleanup temporary directory: %s", strerror(errno));
	}
}

ctest_def_fixture_provider_t__ CTEST_FIXTURE_NAME__(ctest_tmpdir) = {
	&setup__,
	&teardown__,
	sizeof(ctest_tmpdir_fixture_t),
};

int ctest_tmpdir_init(ctest_tmpdir_fixture_t *fixture)
{
	memcpy(fixture->dirname, CTEST_TEMPDIR_PATTERN__, sizeof(fixture->dirname));
	if (mkdtemp(fixture->dirname) == NULL) {
		return -1;
	}
	return 0;
}

int ctest_tmpdir_destroy(ctest_tmpdir_fixture_t *fixture)
{
	int result = 0;
	if (fixture->dirname[0] != '\0') {
		result = deltree(fixture->dirname);
		memset(fixture->dirname, 0, sizeof(fixture->dirname));
	}
	return result;
}
