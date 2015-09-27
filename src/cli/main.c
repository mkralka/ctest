#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <ctest/exec.h>
#include "utils.h"

static const char *self__ = NULL;

/*
 * Command Options
 */

typedef struct command_options command_options_t;
struct command_options {
};

/*
 * Option Parsing
 */

#if 0
static int parse_uint__(unsigned int *p_val, const char *str) {
	char *end;
	unsigned long val;

	errno = 0;

	val = strtoul(str, &end, 0);
	if (errno != 0 || end == str || *end != '\0' || val > UINT_MAX)
		return 1;

	if (p_val != NULL)
		*p_val = (unsigned int)val;
	return 0;
}
#endif

/*
 * Miscellaneous
 */

const char *skip_prefix__(const char *name, const char *prefix, size_t prefix_length) {
	if (strncmp(name, prefix, prefix_length) == 0)
		return name + prefix_length;
	return name;
}

const char *get_testsuite_name__(ctest_testsuite_t *testsuite) {
	return skip_prefix__(ctest_testsuite_get_name(testsuite), "testsuite_", 10);
}

const char *get_test_name__(ctest_test_t *test) {
	return skip_prefix__(ctest_test_get_name(test), "test_", 5);
}

/*
 * Test Suites
 */

typedef struct testsuite_collection testsuite_collection_t;
struct testsuite_collection {
	ctest_testsuite_t **testsuites;
	size_t count;
};

static testsuite_collection_t *load_testsuites__(int argc, char *argv[]) {
	testsuite_collection_t *result;
	ctest_testsuite_t **testsuites = NULL;
	int i;

	if ((result = calloc(1, sizeof(*result))) == NULL) {
		fprintf(stderr, "%s: error allocating memory for test suite collection: %s\n", self__, strerror(errno));
		goto collection_alloc_failed;
	}

	if (argc > 0) {
		if ((testsuites = calloc(argc, sizeof(*testsuites))) == NULL) {
			fprintf(stderr, "%s: error allocating memory for suites: %s\n", self__, strerror(errno));
			goto testsuite_alloc_failed;
		}
	}

	for (i = 0; i < argc; ++i) {
		if ((testsuites[i] = ctest_load_testsuite(argv[i])) == NULL) {
			fprintf(stderr, "%s: error loading suite from %s\n", self__, argv[i]);
			goto load_suite_failed;
		}
	}

	result->testsuites = testsuites;
	result->count = argc;
	return result;

load_suite_failed:
	for (i = 0; i < argc; ++i) {
		if (testsuites[i] != NULL)
			ctest_testsuite_destroy(testsuites[i]);
	}
	free(testsuites);
testsuite_alloc_failed:
	free(result);
collection_alloc_failed:
	return NULL;
}

static void destroy_testsuite_collection__(testsuite_collection_t *collection) {
	size_t i;
	for (i = 0; i < collection->count; ++i) {
		ctest_testsuite_destroy(collection->testsuites[i]);
	}
	free(collection->testsuites);
	memset(collection, 0, sizeof(*collection));
	free(collection);
}

/*
 * run Command
 */

static void run_usage__(FILE *fp)
{
	fprintf(fp,
		"usage: %1$s run [-n] suite [suite [...]]\n"
		"       %1$s run -h\n",
		self__);
}

static void run_help__(FILE *fp)
{
	run_usage__(fp);
	fprintf(fp,
		"\n"
		"Summary:\n"
		"    Run unit tests associated with each unit test suite.\n"
		"\n"
		"    Where <suite> is the module file containing the suite.\n"
		"\n"
		"    For a list of available tests within a suite, see the ls command.\n"
		"\n"
		"Options:\n"
		"    -n          Do not fork child processes to run the test. This is generally\n"
		"                much faster, but may result in a failed tests impacting other\n"
		"                tests. This is useful when running the tests in a debugger or\n"
		"                memory leak detector.\n"
		"    -h          Print this help message.\n"
		"\n");
}

static int run__(command_options_t *unused(options), int argc, char *argv[])
{
	int opt;
	int result = EX_UNAVAILABLE;
	int failure_count;
	bool run_isolated = true;
	ctest_runner_t *runner;
	ctest_reporter_t *reporter;
	testsuite_collection_t *testsuite_collection;

	while ((opt = getopt(argc, argv, "+nh")) != -1) {
		switch (opt) {
		case 'n':
			run_isolated = false;
			break;
		case 'h':
			run_help__(stdout);
			return EX_OK;
		case '?':
		default:
			run_usage__(stderr);
			return EX_USAGE;
		}
	}
	argc -= optind;
	argv += optind;

	if ((testsuite_collection = load_testsuites__(argc, argv)) == NULL) {
		fprintf(stderr, "Error loading test suites: %s\n", strerror(errno));
		goto testsuite_load_failed;
	}

	if ((reporter = ctest_create_console_reporter()) == NULL) {
		fprintf(stderr, "Error creating reporter: %s\n", strerror(errno));
		goto reporter_creation_failed;
	}
	if (run_isolated) {
		runner = ctest_create_forking_runner();
	} else {
		runner = ctest_create_direct_runner();
	}
	if (runner == NULL) {
		fprintf(stderr, "Error creating runner: %s\n", strerror(errno));
		goto runner_creation_failed;
	}

	failure_count = ctest_runner_run_testsuites(runner, reporter, testsuite_collection->testsuites, testsuite_collection->count);
	if (failure_count < 0) {
		fprintf(stderr, "Error running testsuite: %s\n", strerror(errno));
		goto runner_failure;
	}
	if (failure_count == 0)
		result = EX_OK;

runner_failure:
	ctest_runner_destroy(runner);
runner_creation_failed:
	ctest_reporter_destroy(reporter);
reporter_creation_failed:
	destroy_testsuite_collection__(testsuite_collection);
testsuite_load_failed:
	return result;
}

/*
 * ls Command
 */

static void ls_usage__(FILE *fp)
{
	fprintf(fp,
		"usage: %s ls <suite> [<suite> [...]]\n"
		"       %s ls -h\n",
		self__, self__);
}

static void ls_help__(FILE *fp)
{
	ls_usage__(fp);
	fprintf(fp,
		"\n"
		"Summary:\n"
		"    List suites and the associated tests associated with the supplied specs.\n"
		"\n"
		"    Where <suite> is the module file containing the suite.\n"
		"\n"
		"Options:\n"
		"    -h           Print this help message.\n"
		"\n");
}

static int ls__(command_options_t *unused(options), int argc, char *argv[])
{
	size_t i_ts;
	int opt;
	testsuite_collection_t *testsuite_collection;

	while ((opt = getopt(argc, argv, "+h")) != -1) {
		switch (opt) {
		case 'h':
			ls_help__(stdout);
			return 0;
		case '?':
		default:
			ls_usage__(stderr);
			return 1;
		}
	}

	argc -= optind;
	argv += optind;

	if ((testsuite_collection = load_testsuites__(argc, argv)) == NULL) {
		fprintf(stderr, "Error loading test suites: %s\n", strerror(errno));
		return 1;
	}

	for (i_ts = 0; i_ts < testsuite_collection->count; ++i_ts) {
		ctest_testsuite_t *const ts = testsuite_collection->testsuites[i_ts];
		const char *const ts_name = get_testsuite_name__(ts);
		ctest_test_t *const*const tests = ctest_testsuite_get_tests(ts);
		const size_t test_count = ctest_testsuite_get_test_count(ts);
		size_t i_test;
		for (i_test = 0; i_test < test_count; ++i_test) {
			ctest_test_t *const test = tests[i_test];
			printf("%s:%s\n", ts_name, get_test_name__(test));
		}
	}

	destroy_testsuite_collection__(testsuite_collection);
	return 0;
}

/*
 * Main
 */

struct {
	char *name;
	char *description;
	int (*cmd)(command_options_t *options, int argc, char *argv[]);
} commands__[] = {
	{ "run",    "Run unit tests.",              &run__ },
	{ "ls",     "List available unit tests.",   &ls__ },
};

static void print_usage__(FILE *fp)
{
	fprintf(fp,
		"usage: %1$s [options] cmd ...\n"
		"       %1$s cmd -h\n"
		"       %1$s -h\n",
		self__);
}

static void print_help__(FILE *fp)
{
	size_t i;
	int longest_cmd_name = 0;

	for (i = 0; i < countof(commands__); ++i) {
		int len = strlen(commands__[i].name);
		if (len > longest_cmd_name)
			longest_cmd_name = len;
	}
	longest_cmd_name = ((longest_cmd_name + 4) / 4) * 4;
	if (longest_cmd_name < 8)
		longest_cmd_name = 8;

	print_usage__(fp);
	fprintf(fp,
		"\n"
		"Commands:\n");
	for (i = 0; i < countof(commands__); ++i) {
		fprintf(fp, "    %2$-*1$s%3$s\n",
			longest_cmd_name, commands__[i].name, commands__[i].description);
	}
	fprintf(fp,
		"\n"
		"Options:\n"
		"    -h           Print this help message.\n"
		"\n");
}

int main(int argc, char *argv[])
{
	int result = EX_SOFTWARE;
	size_t i;
	int opt;
	command_options_t options;

	self__ = argv[0];
	while ((opt = getopt(argc, argv, "+h")) != -1) {
		switch (opt) {
		case 'h':
			print_help__(stdout);
			return EX_OK;
		case '?':
		default:
			print_usage__(stderr);
			return EX_USAGE;
		}
	}
	argc -= optind;
	argv += optind;

	/* Reset option parsing */
	optind = 1;

	if (argc < 1) {
		/* Nothing was specified, run the first command. */
		char *args[] = { commands__[0].name, NULL };
		result = commands__[0].cmd(&options, 1, args);
	} else {
		for (i = 0; i < countof(commands__); ++i) {
			if (strcmp(argv[0], commands__[i].name) == 0) {
				result = commands__[i].cmd(&options, argc, argv);
				goto done;
			}
		}
		fprintf(stderr, "unknown command: %s\n", argv[0]);
		print_usage__(stderr);
	}
done:
	return result;
}

