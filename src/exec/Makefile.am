include $(top_srcdir)/.automake/buildflags.am
AUTOMAKE_OPTIONS                = foreign 1.4


SUBDIRS                         =

lib_LTLIBRARIES                 = libctestexec.la

libctestexec_la_CPPFLAGS        = $(AM_CPPFLAGS) $(LTDLINCL)
libctestexec_la_SOURCES         = \
                                console_reporter.c \
                                direct_runner.c \
                                exec_events.h exec_events.c \
                                failure.h failure.c \
                                forking_runner.c \
                                loader.c \
                                location.h location.c \
                                output.c \
                                output_reader.h output_reader.c \
                                poll_handler.h \
                                result.c \
                                runner_utils.h runner_utils.c \
                                sig.h sig.c \
                                serialization.h \
                                stacktrace.h stacktrace.c \
                                testing_testsuite.c

libctestexec_la_LIBADD          = $(LIBLTDL)
libctestexec_la_DEPENDENCIES    = $(LTDLDEPS)
