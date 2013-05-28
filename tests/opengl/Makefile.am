bin_PROGRAMS = opengltest
AM_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/include -DOHMD_STATIC $(sdl_CFLAGS) $(GLEW_CFLAGS)
opengltest_SOURCES = gl.c main.c
opengltest_LDADD = $(top_srcdir)/src/libopenhmd.la -lm 
opengltest_LDFLAGS = -static-libtool-libs $(sdl_LIBS) $(GLEW_LIBS) $(GL_LIBS)
