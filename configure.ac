name='openhmd'
version='0.3.0'
email='noname@nurd.se'

AC_PREREQ([2.13])
AC_INIT([openhmd], [0.3.0], [noname@nurd.de])
AM_INIT_AUTOMAKE([foreign -Wall])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AC_CANONICAL_HOST

# Just before release, the LT_VERSION string should be modified. See
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# for details how to increment the current:revision:age fields.
AC_SUBST([LT_VERSION], [1:0:1])

# 0.24 automatically calls AC_SUBST() in PKG_CHECK_MODULES()
PKG_PROG_PKG_CONFIG([0.24])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

hidapi="hidapi"
deps_ld_flags="-lhidapi"
AC_SUBST(hidapi)
AC_SUBST(deps_ld_flags)

AC_SUBST(PKG_CONFIG_EXTRA_PATH, "")
AC_SUBST(EXTRA_LD_FLAGS, "")

AC_MSG_CHECKING([operating system])
AS_CASE(["$host"],
	[*-androideabi],
		[hidapi=""
		deps_ld_flags="-landroid"
		AC_SUBST(EXTRA_LD_FLAGS, "-lm")],
	[*-linux*],
		[AC_MSG_RESULT([$host (Linux)])
		hidapi="hidapi-libusb"
		deps_ld_flags="-lhidapi-libusb"

		#link with realtime lib on linux for clock_gettime, and libm for math functions such as sincosf
		AC_SUBST(EXTRA_LD_FLAGS, "-lrt -lpthread -lm")],
	[*-freebsd*],
		[AC_SUBST(PKG_CONFIG_EXTRA_PATH, "libdata/")
		AC_SUBST(EXTRA_LD_FLAGS, "-lthr")],
	[*-kfreebsd*],
		[AC_SUBST(EXTRA_LD_FLAGS, "-lpthread")]
	[*-darwin*],
		[AC_MSG_RESULT([$host (Mac OS X)])])

# Oculus Rift Driver
AC_ARG_ENABLE([driver-oculus-rift],
        [AS_HELP_STRING([--disable-driver-oculus-rift],
                [disable building of Oculus Rift driver [default=yes]])],
        [driver_oculus_rift_enabled=$enableval],
        [driver_oculus_rift_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_OCULUS_RIFT], [test "x$driver_oculus_rift_enabled" != "xno"])

# HTC Vive driver
AC_ARG_ENABLE([driver-htc-vive],
        [AS_HELP_STRING([--disable-driver-htc-vive],
                [disable building of HTC Vive driver [default=yes]])],
        [driver_htc_vive_enabled=$enableval],
        [driver_htc_vive_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_HTC_VIVE], [test "x$driver_oculus_rift_enabled" != "xno"])

# Deepoon Driver
AC_ARG_ENABLE([driver-deepoon],
        [AS_HELP_STRING([--disable-driver-deepoon],
                [disable building of Deepoon driver [default=yes]])],
        [driver_deepoon_enabled=$enableval],
        [driver_deepoon_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_DEEPOON], [test "x$driver_deepoon_enabled" != "xno"])

# Windows Mixed Reality Driver
AC_ARG_ENABLE([driver-wmr],
        [AS_HELP_STRING([--disable-driver-wmr],
                [disable building of Windows Mixed Reality driver [default=yes]])],
        [driver_wmr_enabled=$enableval],
        [driver_wmr_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_WMR], [test "x$driver_wmr_enabled" != "xno"])

# Sony PSVR Driver, disabled for 0.3
AC_ARG_ENABLE([driver-psvr],
        [AS_HELP_STRING([--disable-driver-psvr],
                [disable building of Sony PSVR driver [default=no]])],
        [driver_psvr_enabled=$enableval],
        [driver_psvr_enabled='no'])

AM_CONDITIONAL([BUILD_DRIVER_PSVR], [test "x$driver_psvr_enabled" != "xno"])

# NOLO
AC_ARG_ENABLE([driver-nolo],
        [AS_HELP_STRING([--disable-driver-nolo],
                [disable building of NOLO VR driver [default=yes]])],
        [driver_nolo_enabled=$enableval],
        [driver_nolo_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_NOLO], [test "x$driver_nolo_enabled" != "xno"])

# 3Glasses HMD driver
AC_ARG_ENABLE([driver-xgvr],
        [AS_HELP_STRING([--disable-driver-xgvr],
                [disable building of 3glasses driver [default=yes]])],
        [driver_xgvr_enabled=$enableval],
        [driver_xgvr_enabled='yes'])

AM_CONDITIONAL([BUILD_DRIVER_XGVR], [test "x$driver_xgvr_enabled" != "xno"])

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

# Libs required by HTC Vive Driver
AS_IF([test "x$driver_htc_vive_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by Depoon Driver
AS_IF([test "x$driver_deepoon_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by Sony PSVR Driver
AS_IF([test "x$driver_psvr_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by NOLO VR Driver
AS_IF([test "x$driver_nolo_enabled" != "xno"],
	[PKG_CHECK_MODULES([hidapi], [$hidapi] >= 0.0.5)])

# Libs required by 3Glasses HMD Driver
AS_IF([test "x$driver_xgvr_enabled" != "xno"],
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
	PKG_CHECK_MODULES([sdl2], [sdl2])

	# Try to find OpenGL with pkg-config
	PKG_CHECK_MODULES([GL], [gl], [],
			# and try to find which lib to link to, -lGL first
			[AC_CHECK_LIB(GL, glBegin, [GL_LIBS=-lGL],

				# if that fails, try -lopengl32 (win32)
				[AC_CHECK_LIB(opengl32, main, [GL_LIBS=-lopengl32],
					[AC_CHECK_HEADERS([OpenGL/gl.h], [GL_LIBS="-framework OpenGL"],
						AC_MSG_ERROR([GL not found])
					)]
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
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile tests/unittests/Makefile examples/Makefile examples/opengl/Makefile examples/simple/Makefile pkg-config/openhmd.pc])
AC_OUTPUT 
