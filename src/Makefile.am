AUTOMAKE_OPTIONS = subdir-objects
library_includedir=$(includedir)/openhmd
library_include_HEADERS = $(top_srcdir)/include/openhmd.h

lib_LTLIBRARIES = libopenhmd.la
libopenhmd_la_SOURCES = \
	openhmd.c \
	platform-win32.c \
	drv_oculus_rift/rift.c \
	drv_oculus_rift/packet.c \
	drv_android/android.c \
	drv_dummy/dummy.c \
	omath.c \
	platform-posix.c \
	fusion.c

libopenhmd_la_LDFLAGS = $(hidapi_LIBS) -no-undefined -version-info 0:0:0 $(EXTRA_LD_FLAGS)
libopenhmd_la_CPPFLAGS = -fPIC -I$(top_srcdir)/include $(hidapi_CFLAGS) -Wall 
