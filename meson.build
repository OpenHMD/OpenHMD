project(
	'openhmd', 'c',
	default_options: 'c_std=c99',
	version: '0.3.0',
	meson_version: '>= 0.44',
)
library_version = '0.1.0'


#
# Dependencies
#

hidapi = 'hidapi'
_hidapi = get_option('hidapi')
if host_machine.system() == 'linux'
	if _hidapi == 'hidraw'
		hidapi = 'hidapi-hidraw'
	else
		hidapi = 'hidapi-libusb'
	endif
else
	if _hidapi != 'auto'
		warning('hidapi option ignored on non-Linux systems')
	endif
endif

dep_libm = meson.get_compiler('c').find_library('m', required: false)
dep_hidapi = dependency(hidapi, required : false)
if not dep_hidapi.found()
	proj_hidapi = subproject('hidapi')
	dep_hidapi = proj_hidapi.get_variable('hidapi_dep')
endif
dep_threads = dependency('threads')

deps = [
	dep_libm,
	dep_threads,
]


#
# Main lib
#

sources = [
	'src/openhmd.c',
	'src/drv_dummy/dummy.c',
	'src/omath.c',
	'src/fusion.c',
	'src/shaders.c',
]
if host_machine.system() == 'windows'
	sources += 'src/platform-win32.c'
else
	sources += 'src/platform-posix.c'
endif
c_args = []
if get_option('default_library') == 'shared'
	if host_machine.system() == 'windows'
		c_args += '-DDLL_EXPORT'
	else
		c_args += '-fvisibility=hidden'
	endif
endif

_drivers = get_option('drivers')
if _drivers.contains('rift')
	sources += [
		'src/drv_oculus_rift/rift.c',
		'src/drv_oculus_rift/packet.c',
	]
	c_args += '-DDRIVER_OCULUS_RIFT'
	deps += dep_hidapi
endif

if _drivers.contains('deepoon')
	sources += [
		'src/drv_deepoon/deepoon.c',
		'src/drv_deepoon/packet.c',
	]
	c_args += '-DDRIVER_DEEPOON'
endif

if _drivers.contains('psvr')
	sources += [
		'src/drv_psvr/psvr.c',
		'src/drv_psvr/packet.c',
	]
	c_args += '-DDRIVER_PSVR'
	deps += dep_hidapi
endif

if _drivers.contains('vive')
	sources += [
		'src/drv_htc_vive/vive.c',
		'src/drv_htc_vive/packet.c',
		'src/ext_deps/nxjson.c',
	]
	c_args += '-DDRIVER_HTC_VIVE'
	deps += dep_hidapi
endif

if _drivers.contains('nolo')
	sources += [
		'src/drv_nolo/nolo.c',
		'src/drv_nolo/packet.c',
	]
	c_args += '-DDRIVER_NOLO'
	deps += dep_hidapi
endif

if _drivers.contains('wmr')
	sources += [
		'src/drv_wmr/wmr.c',
		'src/drv_wmr/packet.c',
		'src/ext_deps/nxjson.c'
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
		'src/drv_external/external.c',
	]
	c_args += '-DDRIVER_EXTERNAL'
endif

if _drivers.contains('android')
	sources += [
		'src/drv_android/android.c',
	]
	c_args += '-DDRIVER_ANDROID'
endif

openhmd_lib = library(
	'openhmd',
	sources,
	include_directories: include_directories('./include'),
	c_args: c_args,
	dependencies: deps,
	install: true,
	version: library_version,
)


#
# Examples
#

_examples = get_option('examples')

# Simple
if _examples.contains('simple')
	deps = [dep_threads]

	simple_sources = [
		'examples/simple/simple.c',
	]

	executable(
		'openhmd_simple_example',
		simple_sources,
		include_directories: include_directories('./include'),
		link_with: [openhmd_lib],
		dependencies: deps,
		install: true,
	)
endif

# OpenGL
if _examples.contains('opengl')

	dep_sdl2 = dependency('sdl2')
	dep_gl = dependency('gl')
	dep_glew = dependency('glew')
	deps = [dep_sdl2, dep_gl, dep_glew, dep_threads]

	opengl_sources = [
		'examples/opengl/main.c',
		'examples/opengl/gl.c',
	]

	executable(
		'openhmd_opengl_example',
		opengl_sources,
		include_directories: include_directories([
			'./include',
			'examples/opengl'
		]),
		link_with: [openhmd_lib],
		dependencies: deps,
		install: true,
	)
endif


#
# Install and pkg-config export file
#

desc = 'API and drivers for immersive technology devices such as HMDs'

pkg = import('pkgconfig')
pkg.generate(
	name: 'openhmd',
	description: desc,
	version: meson.project_version(),
	subdirs: 'openhmd',
	requires: hidapi,
	libraries: openhmd_lib,
	url: 'http://www.openhmd.net/',
)
install_headers('include/openhmd.h', subdir: 'openhmd')
