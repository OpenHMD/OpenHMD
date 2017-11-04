bin_PROGRAMS = unittests
AM_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/include -I$(top_srcdir)/src -DOHMD_STATIC
unittests_SOURCES = main.c quat.c vec.c highlevel.c
unittests_LDADD = $(top_builddir)/src/libopenhmd.la -lm
unittests_LDFLAGS = -static-libtool-libs
