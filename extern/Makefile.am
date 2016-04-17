AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libuvc.la
libuvc_la_SOURCES = \
	libuvc/src/ctrl.c \
	libuvc/src/ctrl-gen.c \
	libuvc/src/device.c \
	libuvc/src/diag.c \
	libuvc/src/frame.c \
	libuvc/src/init.c \
	libuvc/src/stream.c \
	libuvc/src/misc.c

libuvc_la_LDFLAGS = -no-undefined -version-info 0:0:0
libuvc_la_CPPFLAGS = -fPIC -I$(top_srcdir)/extern/libuvc/include -Wall 
libuvc_la_LDFLAGS += $(EXTRA_LD_FLAGS)
