include $(top_srcdir)/.automake/buildflags.am
AUTOMAKE_OPTIONS        = foreign 1.4

SUBDIRS                 =

LDFLAGS                 = -module

examplesdir             = $(pkglibdir)/examples
examples_LTLIBRARIES    = \
        simple_suite.la \
        suite_with_data.la \
        suite_with_fixtures.la

simple_suite_la_SOURCES         = simple_suite.c romnum.h romnum.c
simple_suite_la_LIBADD          = $(top_builddir)/src/tests/libcteststub.la

suite_with_data_la_SOURCES      = suite_with_data.c romnum.h romnum.c
suite_with_data_la_LIBADD       = $(top_builddir)/src/tests/libcteststub.la

suite_with_fixtures_la_SOURCES  = suite_with_fixtures.c hello_world.h hello_world.c
suite_with_fixtures_la_LIBADD   = $(top_builddir)/src/tests/libctest.la \
                                  $(top_builddir)/src/tests/libcteststub.la

TESTS                   = \
        simple_suite.la \
        suite_with_data.la \
        suite_with_fixtures.la
TEST_EXTENSIONS         = .la
LA_LOG_COMPILER         = $(top_builddir)/src/cli/ctester
LA_LOG_FLAGS            = -s
