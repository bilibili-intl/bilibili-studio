cmake_minimum_required(VERSION 3.10)

if(WIN32)
  # windows need winsdk 10.0.19041.0, obs recommend.
  set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
endif(WIN32)

# asm
#enable_language(ASM_NASM)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

# Project name.
project(livehime)

if(WIN32)
  # When building vs projects directly with the "cmake --build xxxxx" command line，
  # By default, the toolchain does not include the atL-related header file containing the directory and library reference directory，
  # Causes projects that use atl to fail to find the reference directory when compiled or linked；
  # Opening the CMake-generated solution for compilation directly with the VS IDE does not have this problem，The IDE will pick up these paths automatically；
  # If you do something like that ${CMAKE_C_COMPILER}（C:/Program Files (x86)/Microsoft Visual Studio/2017/BuildTools/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x86/CL.exe）
  # It's too time and effort to do regular parsing so that the cmake script concatenates the directories itself，
  # The current cmake project is built on the command-line via obs_core project in the vs main project，
  # It is very convenient to integrate VS IDE $(IncludePath)、$(LibraryPath)variable transfer to cmake, save time and effort
  message(STATUS "VS IncludePath: ${VC_INCLUDEPATH}")
  message(STATUS "VS LibraryPath: ${VC_LIBRARYPATH}")
  foreach(_include_item ${VC_INCLUDEPATH})
	  add_compile_options("/I\"${_include_item}\"")
  endforeach()
  foreach(_library_item ${VC_LIBRARYPATH})
	  add_link_options("/LIBPATH:\"${_library_item}\"")
  endforeach()
endif(WIN32)

# cmake path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Introduce common configuration
include(common_config)
include(LivehimeHelpers)

# Read program version number Settings from the current version profile to LIVEHIME_VERSION，
# In order for obsdonfig.h to correctly populate the LIVEHIME_VERSION macro definition
livehime_read_profile("${CMAKE_SOURCE_DIR}/bililive/VERSION" "LIVEHIME")
if(DEFINED ENV{LIVEHIME_MAJOR} AND DEFINED ENV{LIVEHIME_MINOR} AND DEFINED ENV{LIVEHIME_BUILD} AND DEFINED ENV{LIVEHIME_PATCH})
  set(LIVEHIME_FULL_VERSION $ENV{LIVEHIME_MAJOR}.$ENV{LIVEHIME_MINOR}.$ENV{LIVEHIME_BUILD}.$ENV{LIVEHIME_PATCH})
  set(LIVEHIME_VERSION $ENV{LIVEHIME_MAJOR}.$ENV{LIVEHIME_MINOR}.$ENV{LIVEHIME_BUILD})
  message(STATUS "Livehime Version: ${LIVEHIME_FULL_VERSION}")
else()
  message(FATAL_ERROR "bililive/VERSION not exists or data invalid, VERSION must at least contain thess fields:
    MAJOR=4
    MINOR=0
    BUILD=0
    ")
endif()

find_package(Livehime REQUIRED)
find_package(Webrtc REQUIRED)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cef")
  #add_subdirectory(cef)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cef/cef_proxy_dll")
  #add_subdirectory(cef_proxy)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/obs")
  add_subdirectory(obs/obs-studio)
endif()

# Display configuration settings.
#PRINT_COMMON_CONFIG()