name='openhmd'
version='0.0.1'
library_interface_version='0:0:0'
email='noname@nurd.se'

m4_define([OH_VERSION_STRING], [0.0.1])
m4_define([OH_NAME], [openhmd])

AC_PREREQ([2.13])
AC_INIT([OH_NAME], [OH_VERSION_STRING], [noname@nurd.se])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR
LT_INIT
AC_CANONICAL_HOST

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

hidapi="hidapi"

AC_SUBST(PKG_CONFIG_EXTRA_PATH, "")
AC_SUBST(EXTRA_LD_FLAGS, "")

AC_MSG_CHECKING([operating system])
case "$host" in
*-linux*)
	AC_MSG_RESULT([$host (Linux)])
	hidapi="hidapi-libusb"

	#link with realtime lib on linux for clock_gettime
	AC_SUBST(EXTRA_LD_FLAGS, "-lrt")
	;;
*-freebsd*)
	AC_SUBST(PKG_CONFIG_EXTRA_PATH, "libdata/")
	;;
esac

PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)

# Do we build OpenGL example?
AC_ARG_ENABLE([openglexample],
        [AS_HELP_STRING([--enable-openglexample],
                [enable building of OpenGL example (default n)])],
        [openglexample_enabled=$enableval],
        [openglexample_enabled='no'])

AM_CONDITIONAL([BUILD_OPENGL_EXAMPLE], [test "x$openglexample_enabled" != "xno"])

# Libs required by OpenGL test
if test "x$openglexample_enabled" != "xno"; then
	PKG_CHECK_MODULES([sdl], [sdl])

	# Try to find OpenGL with pkg-config
	PKG_CHECK_MODULES([GL], [gl], [], 
			# and try to find which lib to link to, -lGL first
			[AC_CHECK_LIB(GL, glBegin, [GL_LIBS=-lGL], 

				# if that fails, try -lopengl32 (win32)
				[AC_CHECK_LIB(opengl32, main, [GL_LIBS=-lopengl32], 
					AC_MSG_ERROR([GL not found])
				)]
			)]
	)

	AC_SUBST(GL_LIBS)

	# Try to find GLEW with pkg-config
	PKG_CHECK_MODULES([GLEW], [glew], [],
		# if that fails, check if there's a glew header
		[AC_CHECK_HEADER([GL/glew.h], [GLEW_LIBS=-lGLEW; GLEW_CFLAGS=-DGLEW_STATIC], AC_MSG_ERROR([GLEW not found]))]
	)

	AC_SUBST(GLEW_LIBS)
	AC_SUBST(GLEW_CFLAGS)
fi

AC_PROG_CC
AC_PROG_CC_C99

AC_CONFIG_HEADERS([config.h])

AC_OUTPUT([Makefile src/Makefile tests/Makefile tests/unittests/Makefile examples/Makefile examples/opengl/Makefile examples/simple/Makefile])
AC_OUTPUT 
