project(
	'openhmd', 'c',
	default_options : 'c_std=c99',
	version : '0.3.0',
	meson_version : '>= 0.44'
)
library_version = '0.1.0'

sources = [
	'src/openhmd.c',
	'src/platform-win32.c',
	'src/drv_dummy/dummy.c',
	'src/omath.c',
	'src/platform-posix.c',
	'src/fusion.c',
	'src/shaders.c'
]

hidapi = 'hidapi'
if host_machine.system() == 'linux'
	hidapi = 'hidapi-libusb'
endif
dep_hidapi = dependency(hidapi)
deps = [
	meson.get_compiler('c').find_library('m', required : false), #-lm
	dependency('threads') #pthread
]
c_args = []

_drivers = get_option('drivers')
if _drivers.contains('rift')
	sources += [
		'src/drv_oculus_rift/rift.c',
		'src/drv_oculus_rift/packet.c'
	]
	c_args += '-DDRIVER_OCULUS_RIFT'
	deps += dep_hidapi
endif

if _drivers.contains('deepoon')
	sources += [
		'src/drv_deepoon/deepoon.c',
		'src/drv_deepoon/packet.c'
	]
	c_args += '-DDRIVER_DEEPOON'
endif

if _drivers.contains('psvr')
	sources += [
		'src/drv_psvr/psvr.c',
		'src/drv_psvr/packet.c'
	]
	c_args += '-DDRIVER_PSVR'
	deps += dep_hidapi
endif

if _drivers.contains('vive')
	sources += [
		'src/drv_htc_vive/vive.c',
		'src/drv_htc_vive/packet.c',
		'src/ext_deps/nxjson.c'
	]
	c_args += '-DDRIVER_HTC_VIVE'
	deps += dep_hidapi
endif

if _drivers.contains('nolo')
	sources += [
		'src/drv_nolo/nolo.c',
		'src/drv_nolo/packet.c'
	]
	c_args += '-DDRIVER_NOLO'
	deps += dep_hidapi
endif

if _drivers.contains('wmr')
	sources += [
		'src/drv_wmr/wmr.c',
		'src/drv_wmr/packet.c'
	]
	c_args += '-DDRIVER_WMR'
	deps += dep_hidapi
endif

if _drivers.contains('xgvr')
	sources += [
		'src/drv_3glasses/xgvr.c',
		'src/drv_3glasses/packet.c',
	]
	c_args += '-DDRIVER_XGVR'
	deps += dep_hidapi
endif

if _drivers.contains('external')
	sources += [
		'src/drv_external/external.c'
	]
	c_args += '-DDRIVER_EXTERNAL'
endif

if _drivers.contains('android')
	sources += [
		'src/drv_android/android.c'
	]
	c_args += '-DDRIVER_ANDROID'
endif

openhmd_lib = library('openhmd', sources, include_directories : include_directories('./include'), c_args : c_args, dependencies : deps, install : true, version : library_version)

# build examples and install pkg-config file + header only for shared library. shared is the default
if get_option('default_library') == 'shared'
	_examples = get_option('examples')
	if _examples.contains('simple')
		executable('openhmd_simple_example', 'examples/simple/simple.c', include_directories : include_directories('./include'), link_with: [openhmd_lib], install : true)
	endif

	if _examples.contains('opengl')
		opengl_sources = [
			'examples/opengl/main.c',
			'examples/opengl/gl.c',
		]
		sdldep = dependency('sdl2')
		gldep = dependency('gl')
		glewdep = dependency('glew')
		executable('openhmd_opengl_example', opengl_sources , include_directories : include_directories(['./include', 'examples/opengl']), link_with: [openhmd_lib], dependencies : [sdldep, gldep, glewdep], install : true)
	endif
	pkg = import('pkgconfig')
	pkg.generate(
		name : 'openhmd',
		description : 'API and drivers for immersive technology devices such as HMDs',
		version : meson.project_version(),
		subdirs : 'openhmd',
		requires : hidapi,
		libraries : openhmd_lib,
		url : 'http://www.openhmd.net/'
	)
	install_headers('include/openhmd.h', subdir : 'openhmd')
endif
