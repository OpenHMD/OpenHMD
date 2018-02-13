project('openhmd', 'c', default_options : 'c_std=c99')

cc = meson.get_compiler('c')

sources = [
	'src/openhmd.c',
	'src/platform-win32.c',
	'src/drv_dummy/dummy.c',
	'src/omath.c',
	'src/platform-posix.c',
	'src/fusion.c',
	'src/shaders.c'
]

deps = [
	cc.find_library('m', required : false), #-lm
	dependency('threads') #pthread
]
c_args = []

if get_option('OPENHMD_DRIVER_OCULUS_RIFT')
	sources += [
		'src/drv_oculus_rift/rift.c',
		'src/drv_oculus_rift/packet.c'
	]
	c_args += '-DDRIVER_OCULUS_RIFT'
	deps += dependency('hidapi-libusb')
endif

if get_option('OPENHMD_DRIVER_DEEPOON')
	sources += [
		'src/drv_deepoon/deepoon.c',
		'src/drv_deepoon/packet.c'
	]
	c_args += '-DDRIVER_DEEPOON'
endif

if get_option('OPENHMD_DRIVER_PSVR')
	sources += [
		'src/drv_psvr/psvr.c',
		'src/drv_psvr/packet.c'
	]
	c_args += '-DDRIVER_PSVR'
	deps += dependency('hidapi-libusb')
endif

if get_option('OPENHMD_DRIVER_HTC_VIVE')
	sources += [
		'src/drv_htc_vive/vive.c',
		'src/drv_htc_vive/packet.c',
		'src/ext_deps/mjson.c'
	]
	c_args += '-DDRIVER_HTC_VIVE'
	deps += dependency('hidapi-libusb')
endif

if get_option('OPENHMD_DRIVER_NOLO')
	sources += [
		'src/drv_nolo/nolo.c',
		'src/drv_nolo/packet.c'
	]
	c_args += '-DDRIVER_NOLO'
	deps += dependency('hidapi-libusb')
endif

if get_option('OPENHMD_DRIVER_EXTERNAL')
	sources += [
		'src/drv_external/external.c'
	]
	c_args += '-DDRIVER_EXTERNAL'
endif

if get_option('OPENHMD_DRIVER_ANDROID')
	sources += [
		'src/drv_android/android.c'
	]
	c_args += '-DDRIVER_ANDROID'
endif

openhmd_lib = shared_library('openhmd', sources, include_directories : include_directories('./include'), dependencies : deps, install : true)

install_headers('include/openhmd.h')

simple_example = get_option('OPENHMD_EXAMPLE_SIMPLE')
if simple_example
	executable('openhmd_simple_example', 'examples/simple/simple.c', include_directories : include_directories('./include'), link_with: [openhmd_lib], install : true)
endif

sdl_example = get_option('OPENHMD_EXAMPLE_SDL')
if sdl_example
	opengl_sources = [
		'examples/opengl/main.c',
		'examples/opengl/gl.c',
	]
	sdldep = dependency('sdl2')
	glewdep = dependency('glew')
	executable('openhmd_opengl_example', opengl_sources , include_directories : include_directories(['./include', 'examples/opengl']), link_with: [openhmd_lib], dependencies : [sdldep, glewdep], install : true)
endif
