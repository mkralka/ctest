include $(top_srcdir)/.automake/buildflags.am
AUTOMAKE_OPTIONS                = foreign 1.4

SUBDIRS                         =

# includes go first, everything depends on this
SUBDIRS                        += include

# Libs (mostly independenty) follow, in alpha ordering
SUBDIRS                        += exec tests

# The CLI depends on libs
SUBDIRS                        += cli

