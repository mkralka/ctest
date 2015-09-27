include $(top_srcdir)/.automake/buildflags.am
AUTOMAKE_OPTIONS                = foreign 1.4

SUBDIRS                         = fixtures

lib_LTLIBRARIES                 = libctest.la libcteststub.la

libctest_la_LIBADD              = fixtures/libctestfixtures.la
libctest_la_SOURCES             =
libctest_la_CFLAGS              = -prefer-pic $(AM_CFLAGS)
libctest_la_LDFLAGS             = -static

libcteststub_la_SOURCES         = stub.c
libcteststub_la_CFLAGS          = -prefer-pic $(AM_CFLAGS)
libcteststub_la_LDFLAGS         = -static
