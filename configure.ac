name='openhmd'
version='0.0.1'
library_interface_version='0:0:0'
email='noname@nurd.se'

AC_PREREQ([2.13])
AC_INIT([openhmd], [0.0.1], [noname@nurd.se])
AM_INIT_AUTOMAKE([foreign -Wall])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AC_CANONICAL_HOST

# 0.24 automatically calls AC_SUBST() in PKG_CHECK_MODULES()
PKG_PROG_PKG_CONFIG([0.24])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

hidapi="hidapi"

AC_SUBST(PKG_CONFIG_EXTRA_PATH, "")
AC_SUBST(EXTRA_LD_FLAGS, "")

AC_MSG_CHECKING([operating system])
AS_CASE(["$host"],
	[*-linux*],
		[AC_MSG_RESULT([$host (Linux)])
		hidapi="hidapi-libusb"

		#link with realtime lib on linux for clock_gettime
		AC_SUBST(EXTRA_LD_FLAGS, "-lrt -lpthread")],
	[*-freebsd*],
		[AC_SUBST(PKG_CONFIG_EXTRA_PATH, "libdata/")])

# Oculus Rift Driver
AC_ARG_ENABLE([driver-oculus-rift],
        [AS_HELP_STRING([--disable-driver-oculus-rift],
                [disable building of Oculus Rift driver [default=yes]])],
        [driver_oculus_rift_enabled=$enableval],
        [driver_oculus_rift_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_OCULUS_RIFT], [test "x$driver_oculus_rift_enabled" != "xno"])

# Deepoon Driver
AC_ARG_ENABLE([driver-deepoon],
        [AS_HELP_STRING([--disable-driver-deepoon],
                [disable building of Deepoon driver [default=yes]])],
        [driver_deepoon_enabled=$enableval],
        [driver_deepoon_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_DEEPOON], [test "x$driver_deepoon_enabled" != "xno"])

# Sony PSVR Driver
AC_ARG_ENABLE([driver-psvr],
        [AS_HELP_STRING([--disable-driver-psvr],
                [disable building of Sony PSVR driver [default=yes]])],
        [driver_psvr_enabled=$enableval],
        [driver_psvr_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_PSVR], [test "x$driver_psvr_enabled" != "xno"])

# External Driver
AC_ARG_ENABLE([driver-external],
        [AS_HELP_STRING([--disable-driver-external],
                [disable building of External driver [default=yes]])],
        [driver_external_enabled=$enableval],
        [driver_external_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_EXTERNAL], [test "x$driver_external_enabled" != "xno"])

# Android Driver
AC_ARG_ENABLE([driver-android],
        [AS_HELP_STRING([--enable-driver-android],
                [enable building of Android driver [default=no]])],
        [driver_android_enabled=$enableval],
        [driver_android_enabled='no'])

AM_CONDITIONAL([BUILD_DRIVER_ANDROID], [test "x$driver_android_enabled" != "xno"])

# Libs required by Oculus Rift Driver
AS_IF([test "x$driver_oculus_rift_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by Depoon Driver
AS_IF([test "x$driver_deepoon_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by Sony PSVR Driver
AS_IF([test "x$driver_psvr_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Do we build OpenGL example?
AC_ARG_ENABLE([openglexample],
        [AS_HELP_STRING([--enable-openglexample],
                [enable building of OpenGL example [default=no]])],
        [openglexample_enabled=$enableval],
        [openglexample_enabled='no'])

AM_CONDITIONAL([BUILD_OPENGL_EXAMPLE], [test "x$openglexample_enabled" != "xno"])

# Libs required by OpenGL test
AS_IF([test "x$openglexample_enabled" != "xno"], [
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
])

AC_PROG_CC
AC_PROG_CC_C99

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile tests/unittests/Makefile examples/Makefile examples/opengl/Makefile examples/simple/Makefile])
AC_OUTPUT