cmake_minimum_required(VERSION 3.1)

project(openhmd C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 1)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION_STRING ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(BUILD_BOTH_STATIC_SHARED_LIBS OFF)

#source files set just for Android
set(openhmd_source_files
	${CMAKE_CURRENT_LIST_DIR}/src/openhmd.c
	${CMAKE_CURRENT_LIST_DIR}/src/platform-win32.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_dummy/dummy.c
	${CMAKE_CURRENT_LIST_DIR}/src/omath.c
	${CMAKE_CURRENT_LIST_DIR}/src/platform-posix.c
	${CMAKE_CURRENT_LIST_DIR}/src/fusion.c
	${CMAKE_CURRENT_LIST_DIR}/src/shaders.c
)

option(OPENHMD_DRIVER_OCULUS_RIFT "Oculus Rift DK1 and DK2" ON)
option(OPENHMD_DRIVER_DEEPOON "Deepoon E2" ON)
option(OPENHMD_DRIVER_WMR "Windows Mixed Reality" ON)
option(OPENHMD_DRIVER_PSVR "Sony PSVR" OFF)
option(OPENHMD_DRIVER_HTC_VIVE "HTC Vive" ON)
option(OPENHMD_DRIVER_NOLO "NOLO VR CV1" ON)
option(OPENHMD_DRIVER_XGVR "3Glasses HMD" ON)
option(OPENHMD_DRIVER_EXTERNAL "External sensor driver" ON)
option(OPENHMD_DRIVER_ANDROID "General Android driver" OFF)

option(OPENHMD_EXAMPLE_SIMPLE "Simple test binary" ON)
option(OPENHMD_EXAMPLE_SDL "SDL OpenGL test (outdated)" OFF)

if(OPENHMD_DRIVER_OCULUS_RIFT)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_oculus_rift/rift.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_oculus_rift/packet.c
	)
	add_definitions(-DDRIVER_OCULUS_RIFT)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_OCULUS_RIFT)

if(OPENHMD_DRIVER_DEEPOON)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_deepoon/deepoon.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_deepoon/packet.c
	)
	add_definitions(-DDRIVER_DEEPOON)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_DEEPOON)

if(OPENHMD_DRIVER_WMR)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_wmr/wmr.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_wmr/packet.c
	)
	add_definitions(-DDRIVER_WMR)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_WMR)

if(OPENHMD_DRIVER_PSVR)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_psvr/psvr.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_psvr/packet.c
	)
	add_definitions(-DDRIVER_PSVR)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_PSVR)

if(OPENHMD_DRIVER_HTC_VIVE)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_htc_vive/vive.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_htc_vive/packet.c
	#${CMAKE_CURRENT_LIST_DIR}/src/ext_deps/miniz.c
	${CMAKE_CURRENT_LIST_DIR}/src/ext_deps/nxjson.c
	)
	add_definitions(-DDRIVER_HTC_VIVE)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_HTC_VIVE)

if(OPENHMD_DRIVER_NOLO)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_nolo/nolo.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_nolo/packet.c
	)
	add_definitions(-DDRIVER_NOLO)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_NOLO)

if(OPENHMD_DRIVER_XGVR)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_3glasses/xgvr.c
	${CMAKE_CURRENT_LIST_DIR}/src/drv_3glasses/packet.c
	)
	add_definitions(-DDRIVER_XGVR)

	find_package(HIDAPI REQUIRED)
	include_directories(${HIDAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})
endif(OPENHMD_DRIVER_XGVR)

if (OPENHMD_DRIVER_EXTERNAL)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_external/external.c
	)
	add_definitions(-DDRIVER_EXTERNAL)
endif(OPENHMD_DRIVER_EXTERNAL)

if (OPENHMD_DRIVER_ANDROID)
	set(openhmd_source_files ${openhmd_source_files}
	${CMAKE_CURRENT_LIST_DIR}/src/drv_android/android.c
	)
	add_definitions(-DDRIVER_ANDROID)
endif(OPENHMD_DRIVER_ANDROID)

if (OPENHMD_EXAMPLE_SIMPLE)
	add_subdirectory(./examples/simple)
endif(OPENHMD_EXAMPLE_SIMPLE)

if (OPENHMD_EXAMPLE_SDL)
	find_package(SDL2 REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(OpenGL REQUIRED)
	add_subdirectory(./examples/opengl)
endif (OPENHMD_EXAMPLE_SDL)

set(TARGETS "")

if (BUILD_BOTH_STATIC_SHARED_LIBS)

	# Shared
	add_library(openhmd-shared SHARED ${openhmd_source_files})
	set_target_properties(openhmd-shared PROPERTIES OUTPUT_NAME openhmd CLEAN_DIRECT_OUTPUT 1)

	# Static
	add_library(openhmd-static STATIC ${openhmd_source_files})
	set_target_properties(openhmd-static PROPERTIES OUTPUT_NAME openhmd CLEAN_DIRECT_OUTPUT 1)

	# Alias needed to compile examples
	add_library(openhmd ALIAS openhmd-shared)

	set(TARGETS "openhmd-shared" "openhmd-static")

else ()

	# Static or Shared
	add_library(openhmd ${openhmd_source_files})

	export(TARGETS openhmd FILE openhmd-config.cmake)

	set(TARGETS "openhmd")

endif ()

foreach(target ${TARGETS})

	set_target_properties(${target} PROPERTIES VERSION ${LIB_VERSION_STRING} SOVERSION ${LIB_VERSION_MAJOR})
	target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_link_libraries(${target} ${LIBS})
	if (UNIX AND NOT APPLE)
		target_link_libraries(${target} rt)
	endif()
	if (UNIX)
		target_link_libraries(${target} pthread)
	endif (UNIX)

	get_target_property(target_type ${target} TYPE)
	if (target_type STREQUAL "STATIC_LIBRARY")
	    target_compile_definitions(${target} PUBLIC -DOHMD_STATIC)
	endif()

endforeach(target)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/openhmd.pc.cmakein"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

install(TARGETS ${TARGETS} DESTINATION lib)
install(FILES include/openhmd.h DESTINATION include)
install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION lib/pkgconfig)
