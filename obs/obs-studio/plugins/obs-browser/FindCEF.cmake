include(FindPackageHandleStandardArgs)

# modify [
#SET(CEF_ROOT_DIR "" CACHE PATH "Path to a CEF distributed build")
SET(CEF_ROOT_INCLUDE_DIR "${OBS_STUDIO_SOURCE_DIR}/../../cef/cef_core" CACHE PATH "Path to a CEF distributed build")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
   SET(CEF_ROOT_DIR "${OBS_STUDIO_SOURCE_DIR}/../../cef/cef_core/x64" CACHE PATH "Path to a CEF distributed build")
else()
   SET(CEF_ROOT_DIR "${OBS_STUDIO_SOURCE_DIR}/../../cef/cef_core/Win32" CACHE PATH "Path to a CEF distributed build")
endif()
# ]

message(STATUS "Looking for Chromium Embedded Framework in ${CEF_ROOT_DIR}")

find_path(CEF_INCLUDE_DIR "include/cef_version.h"
	HINTS ${CEF_ROOT_INCLUDE_DIR})

if(APPLE)
	find_library(CEF_LIBRARY
		NAMES cef libcef cef.lib libcef.o "Chromium Embedded Framework"
		NO_DEFAULT_PATH
		PATHS ${CEF_ROOT_DIR} ${CEF_ROOT_DIR}/Release)
	find_library(CEFWRAPPER_LIBRARY
		NAMES cef_dll_wrapper libcef_dll_wrapper
		NO_DEFAULT_PATH
		PATHS ${CEF_ROOT_DIR}/build/libcef_dll/Release
			${CEF_ROOT_DIR}/build/libcef_dll_wrapper/Release
			${CEF_ROOT_DIR}/build/libcef_dll
			${CEF_ROOT_DIR}/build/libcef_dll_wrapper)
	# modify [
	find_library(CEFWRAPPER_LIBRARY_DEBUG
		NAMES cef_dll_wrapper libcef_dll_wrapper
		NO_DEFAULT_PATH
		PATHS ${CEF_ROOT_DIR}/build/libcef_dll/Debug
			${CEF_ROOT_DIR}/build/libcef_dll_wrapper/Debug)
	# ]
elseif(UNIX)
	find_library(CEF_LIBRARY
		NAMES libcef.so "Chromium Embedded Framework"
		NO_DEFAULT_PATH
		PATHS ${CEF_ROOT_DIR} ${CEF_ROOT_DIR}/Release)
	find_library(CEFWRAPPER_LIBRARY
		NAMES libcef_dll_wrapper.a
		NO_DEFAULT_PATH
		PATHS ${CEF_ROOT_DIR}/build/libcef_dll_wrapper
			${CEF_ROOT_DIR}/libcef_dll_wrapper)
else()
	find_library(CEF_LIBRARY
		NAMES cef libcef cef.lib libcef.o "Chromium Embedded Framework"
		PATHS ${CEF_ROOT_DIR} ${CEF_ROOT_DIR}/Release)
	find_library(CEFWRAPPER_LIBRARY
		NAMES cef_dll_wrapper libcef_dll_wrapper
		PATHS ${CEF_ROOT_DIR}/build/libcef_dll/Release
			${CEF_ROOT_DIR}/build/libcef_dll_wrapper/Release
			${CEF_ROOT_DIR}/build/libcef_dll
			${CEF_ROOT_DIR}/build/libcef_dll_wrapper)
	if(WIN32)
		find_library(CEFWRAPPER_LIBRARY_DEBUG
			NAMES cef_dll_wrapper libcef_dll_wrapper
			PATHS ${CEF_ROOT_DIR}/build/libcef_dll/Debug ${CEF_ROOT_DIR}/build/libcef_dll_wrapper/Debug)
	endif()
endif()

if(NOT CEF_LIBRARY)
	message(WARNING "Could not find the CEF shared library" )
	set(CEF_FOUND FALSE)
	return()
endif()

if(NOT CEFWRAPPER_LIBRARY)
	message(WARNING "Could not find the CEF wrapper library" )
	set(CEF_FOUND FALSE)
	return()
endif()

# modify [
message(STATUS "Found CEF_INCLUDE_DIR in '${CEF_INCLUDE_DIR}'")
message(STATUS "Found CEF_LIBRARY in '${CEF_LIBRARY}'")
message(STATUS "Found CEFWRAPPER_LIBRARY in '${CEFWRAPPER_LIBRARY}'")
message(STATUS "Found CEFWRAPPER_LIBRARY_DEBUG in '${CEFWRAPPER_LIBRARY_DEBUG}'")

if(APPLE)
	# https://cmake.org/cmake/help/latest/command/find_library.html
	# If the library found is a framework, then <VAR> will be set to the full path to the framework <fullPath>/A.framework. 
	# When a full path to a framework is used as a library, CMake will use a -framework A, and a -F<fullPath> to link the framework to the target.
	# xcode's linker flags command not allow path witch contain space, so "Chromium Embedded Framework" will be treat as three parts "Chromium" "Embedded" "Framework" ,
	# and project will build failed cause "can't found framework"
	function(link_cef_framework target)
		get_filename_component(_fw_dir ${CEF_LIBRARY} DIRECTORY)
		get_filename_component(_fw_name ${CEF_LIBRARY} NAME_WE)
		string(REPLACE " " "\\ " _fw ${_fw_name})
		set_property(TARGET ${target} PROPERTY LINK_FLAGS "-F ${_fw_dir} -framework ${_fw}")
	endfunction()
endif()
# ]

if(WIN32)
	set(CEF_LIBRARIES
			${CEF_LIBRARY}
			optimized ${CEFWRAPPER_LIBRARY})
	if (CEFWRAPPER_LIBRARY_DEBUG)
		list(APPEND CEF_LIBRARIES
				debug ${CEFWRAPPER_LIBRARY_DEBUG})
	endif()
else()
	# Fixes cmake 3.19.0 commit that added support for modern Xcode build system, but "forgot"
	# to also escape framework names themselves in addition to the framework path:
	# Commit https://gitlab.kitware.com/cmake/cmake/-/commit/ce2dee9e5bae37c8117087bb83add075c3c123b4
	if(${CMAKE_VERSION} VERSION_GREATER "3.19.0" AND XCODE)
		# modify [
		#string(REPLACE "Chromium Embedded Framework" "\"Chromium Embedded Framework\"" CEF_LIBRARY_FIXED ${CEF_LIBRARY})
		#set(CEF_LIBRARIES
		#		${CEF_LIBRARY_FIXED}
		#		${CEFWRAPPER_LIBRARY})
				
		# mac not support optimized/debug
		set(CEF_LIBRARIES
			$<$<CONFIG:Debug>:${CEFWRAPPER_LIBRARY_DEBUG}>
			$<$<CONFIG:Release>:${CEFWRAPPER_LIBRARY}>)
		# ]
	else()
		set(CEF_LIBRARIES
				${CEF_LIBRARY}
				${CEFWRAPPER_LIBRARY})
	endif()
endif()

find_package_handle_standard_args(CEF DEFAULT_MSG CEF_LIBRARY
	CEFWRAPPER_LIBRARY CEF_INCLUDE_DIR)
mark_as_advanced(CEF_LIBRARY CEF_WRAPPER_LIBRARY CEF_LIBRARIES
	CEF_INCLUDE_DIR)
