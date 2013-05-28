bin_PROGRAMS = simple
AM_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/include -DOHMD_STATIC
simple_SOURCES = simple.c
simple_LDADD = $(top_srcdir)/src/libopenhmd.la -lm
simple_LDFLAGS = -static-libtool-libs
