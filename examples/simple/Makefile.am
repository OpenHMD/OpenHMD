bin_PROGRAMS = simple
AM_CPPFLAGS = -Wall -I$(top_srcdir)/include -DOHMD_STATIC
simple_SOURCES = simple.c
simple_LDADD = $(top_builddir)/src/libopenhmd.la -lm
simple_LDFLAGS = -static-libtool-libs
