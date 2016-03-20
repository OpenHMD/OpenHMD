bin_PROGRAMS = openglexample
AM_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/include -DOHMD_STATIC $(sdl_CFLAGS) $(GLEW_CFLAGS)
openglexample_SOURCES = gl.c main.c
openglexample_LDADD = $(top_builddir)/src/libopenhmd.la -lm
openglexample_LDFLAGS = -static-libtool-libs $(sdl_LIBS) $(GLEW_LIBS) $(GL_LIBS)
