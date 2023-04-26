# Copy from cef/cmake/cef_variables.cmake

#
# Shared configuration.
#

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

# Determine the project architecture.
if(NOT DEFINED PROJECT_ARCH)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PROJECT_ARCH "x86_64")
  else()
    set(PROJECT_ARCH "x86")
  endif()

  if(OS_MACOSX)
    # PROJECT_ARCH should be specified on Mac OS X.
    message(STATUS "No PROJECT_ARCH value specified, using ${PROJECT_ARCH}")
  endif()
endif()

if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  set(GEN_NINJA 1)
elseif(${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  set(GEN_MAKEFILES 1)
endif()

# Determine the build type.
if(NOT CMAKE_BUILD_TYPE AND (GEN_NINJA OR GEN_MAKEFILES))
  # CMAKE_BUILD_TYPE should be specified when using Ninja or Unix Makefiles.
  set(CMAKE_BUILD_TYPE Release)
  message(WARNING "No CMAKE_BUILD_TYPE value selected, using ${CMAKE_BUILD_TYPE}")
endif()

# Shared compiler/linker flags.
list(APPEND COMMON_COMPILER_DEFINES
  # Allow C++ programs to use stdint.h macros specified in the C99 standard that aren't
  # in the C++ standard (e.g. UINT8_MAX, INT64_MIN, etc)
  __STDC_CONSTANT_MACROS __STDC_FORMAT_MACROS
  )

#
# Linux configuration.
#
if(OS_LINUX)
  # Platform-specific compiler/linker flags.
  set(COMMON_LIBTYPE SHARED)
  list(APPEND COMMON_COMPILER_FLAGS
    -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
    -fPIC                           # Generate position-independent code for shared libraries
    -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
    -funwind-tables                 # Support stack unwinding for backtrace()
    -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
    --param=ssp-buffer-size=4       # Set the minimum buffer size protected by SSP (security feature, related to stack-protector)
    -pipe                           # Use pipes rather than temporary files for communication between build stages
    -pthread                        # Use the pthread library
    -Wall                           # Enable all warnings
    -Werror                         # Treat warnings as errors
    -Wno-missing-field-initializers # Don't warn about missing field initializers
    -Wno-unused-parameter           # Don't warn about unused parameters
    -Wno-error=comment              # Don't warn about code in comments
    -Wno-comment                    # Don't warn about code in comments
    )
  list(APPEND COMMON_C_COMPILER_FLAGS
    -std=c99                        # Use the C99 language standard
    )
  list(APPEND COMMON_CXX_COMPILER_FLAGS
    -fno-exceptions                 # Disable exceptions
    -fno-rtti                       # Disable real-time type information
    -fno-threadsafe-statics         # Don't generate thread-safe statics
    -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
    -std=gnu++11                    # Use the C++11 language standard including GNU extensions
    -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
    )
  list(APPEND COMMON_COMPILER_FLAGS_DEBUG
    -O0                             # Disable optimizations
    -g                              # Generate debug information
    )
  list(APPEND COMMON_COMPILER_FLAGS_RELEASE
    -O2                             # Optimize for maximum speed
    -fdata-sections                 # Enable linker optimizations to improve locality of reference for data sections
    -ffunction-sections             # Enable linker optimizations to improve locality of reference for function sections
    -fno-ident                      # Ignore the #ident directive
    -U_FORTIFY_SOURCE               # Undefine _FORTIFY_SOURCE in case it was previously defined
    -D_FORTIFY_SOURCE=2             # Add memory and string function protection (security feature, related to stack-protector)
    )
  list(APPEND COMMON_LINKER_FLAGS
    -fPIC                           # Generate position-independent code for shared libraries
    -pthread                        # Use the pthread library
    -Wl,--disable-new-dtags         # Don't generate new-style dynamic tags in ELF
    -Wl,--fatal-warnings            # Treat warnings as errors
    -Wl,-rpath,.                    # Set rpath so that libraries can be placed next to the executable
    -Wl,-z,noexecstack              # Mark the stack as non-executable (security feature)
    -Wl,-z,now                      # Resolve symbols on program start instead of on first use (security feature)
    -Wl,-z,relro                    # Mark relocation sections as read-only (security feature)
    )
  list(APPEND COMMON_LINKER_FLAGS_RELEASE
    -Wl,-O1                         # Enable linker optimizations
    -Wl,--as-needed                 # Only link libraries that export symbols used by the binary
    -Wl,--gc-sections               # Remove unused code resulting from -fdata-sections and -function-sections
    )
  list(APPEND COMMON_COMPILER_DEFINES
    _FILE_OFFSET_BITS=64            # Allow the Large File Support (LFS) interface to replace the old interface
    )
  list(APPEND COMMON_COMPILER_DEFINES_DEBUG
    DEBUG                           # a debug build
    )
  list(APPEND COMMON_COMPILER_DEFINES_RELEASE
    NDEBUG                          # Not a debug build
    )

  include(CheckCCompilerFlag)
  include(CheckCXXCompilerFlag)

  CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
  if(COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
    list(APPEND COMMON_CXX_COMPILER_FLAGS
      -Wno-undefined-var-template   # Don't warn about potentially uninstantiated static members
      )
  endif()

  CHECK_C_COMPILER_FLAG(-Wno-unused-local-typedefs COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
  if(COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
    list(APPEND COMMON_C_COMPILER_FLAGS
      -Wno-unused-local-typedefs  # Don't warn about unused local typedefs
      )
  endif()

  CHECK_CXX_COMPILER_FLAG(-Wno-literal-suffix COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
  if(COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
    list(APPEND COMMON_CXX_COMPILER_FLAGS
      -Wno-literal-suffix         # Don't warn about invalid suffixes on literals
      )
  endif()

  CHECK_CXX_COMPILER_FLAG(-Wno-narrowing COMPILER_SUPPORTS_NO_NARROWING)
  if(COMPILER_SUPPORTS_NO_NARROWING)
    list(APPEND COMMON_CXX_COMPILER_FLAGS
      -Wno-narrowing              # Don't warn about type narrowing
      )
  endif()

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND COMMON_CXX_COMPILER_FLAGS
      -Wno-attributes             # The cfi-icall attribute is not supported by the GNU C++ compiler
      )
  endif()

  if(PROJECT_ARCH STREQUAL "x86_64")
    # 64-bit architecture.
    list(APPEND COMMON_COMPILER_FLAGS
      -m64
      -march=x86-64
      )
    list(APPEND COMMON_LINKER_FLAGS
      -m64
      )
  elseif(PROJECT_ARCH STREQUAL "x86")
    # 32-bit architecture.
    list(APPEND COMMON_COMPILER_FLAGS
      -msse2
      -mfpmath=sse
      -mmmx
      -m32
      )
    list(APPEND COMMON_LINKER_FLAGS
      -m32
      )
  endif()

  # Standard libraries.
  set(COMMON_STANDARD_LIBS
    X11
    )
endif()

#
# Mac OS X configuration.
#
if(OS_MACOSX)
  # Platform-specific compiler/linker flags.
  # See also Xcode target properties in cef_macros.cmake.
  set(COMMON_LIBTYPE SHARED)
  list(APPEND COMMON_COMPILER_FLAGS
    -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
    -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
    -funwind-tables                 # Support stack unwinding for backtrace()
    -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
    -Wall                           # Enable all warnings
    -Werror                         # Treat warnings as errors
    -Wextra                         # Enable additional warnings
    -Wendif-labels                  # Warn whenever an #else or an #endif is followed by text
    -Wnewline-eof                   # Warn about no newline at end of file
    -Wno-missing-field-initializers # Don't warn about missing field initializers
    -Wno-unused-parameter           # Don't warn about unused parameters
    )
  list(APPEND COMMON_C_COMPILER_FLAGS
    -std=c99                        # Use the C99 language standard
    )
  list(APPEND COMMON_CXX_COMPILER_FLAGS
    -fno-exceptions                 # Disable exceptions
    -fno-rtti                       # Disable real-time type information
    -fno-threadsafe-statics         # Don't generate thread-safe statics
    -fobjc-call-cxx-cdtors          # Call the constructor/destructor of C++ instance variables in ObjC objects
    -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
    -std=gnu++11                    # Use the C++11 language standard including GNU extensions
    -Wno-narrowing                  # Don't warn about type narrowing
    -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
    )
  list(APPEND COMMON_COMPILER_FLAGS_DEBUG
    -O0                             # Disable optimizations
    -g                              # Generate debug information
    )
  list(APPEND COMMON_COMPILER_FLAGS_RELEASE
    -O3                             # Optimize for maximum speed plus a few extras
    -g                              # Release also Generate debug information
    )
  list(APPEND COMMON_LINKER_FLAGS
    -Wl,-search_paths_first         # Search for static or shared library versions in the same pass
    -Wl,-ObjC                       # Support creation of ObjC static libraries
    -Wl,-pie                        # Generate position-independent code suitable for executables only
    )
  list(APPEND COMMON_LINKER_FLAGS_RELEASE
    -Wl,-dead_strip                 # Strip dead code
    )

  include(CheckCXXCompilerFlag)

  CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
  if(COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
    list(APPEND COMMON_CXX_COMPILER_FLAGS
      -Wno-undefined-var-template   # Don't warn about potentially uninstantiated static members
      )
  endif()

  # Standard libraries.
  set(COMMON_STANDARD_LIBS
    -lpthread
    "-framework Cocoa"
    "-framework AppKit"
    )

  # Find the newest available base SDK.
  execute_process(COMMAND xcode-select --print-path OUTPUT_VARIABLE XCODE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
  foreach(OS_VERSION 10.11 10.10 10.9)
    set(SDK "${XCODE_PATH}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${OS_VERSION}.sdk")
    if(NOT "${CMAKE_OSX_SYSROOT}" AND EXISTS "${SDK}" AND IS_DIRECTORY "${SDK}")
      set(CMAKE_OSX_SYSROOT ${SDK})
    endif()
  endforeach()

  # Target SDK.
  # obs-studio recommand deployment_target 10.13,
  # but cef-75.1.16 need less than 10.11, because cef use Deprecated atomic function invocation on MacOS Sierra(10.12)
  set(COMMON_TARGET_SDK               "10.11")
  list(APPEND COMMON_COMPILER_FLAGS
    -mmacosx-version-min=${COMMON_TARGET_SDK}
  )
  set(CMAKE_OSX_DEPLOYMENT_TARGET  ${COMMON_TARGET_SDK})

  # Target architecture.
  if(PROJECT_ARCH STREQUAL "x86_64")
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
  else()
    set(CMAKE_OSX_ARCHITECTURES "i386")
  endif()

  # Prevent Xcode 11 from doing automatic codesigning.
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

  # CEF Helper app suffixes.
  # Format is "<name suffix>:<target suffix>:<plist suffix>".
  set(COMMON_HELPER_APP_SUFFIXES
    "::"
    " (GPU):_gpu:.gpu"
    " (Plugin):_plugin:.plugin"
    " (Renderer):_renderer:.renderer"
    )
endif()

#
# Windows configuration.
#
if(OS_WINDOWS)
  if (GEN_NINJA)
    # When using the Ninja generator clear the CMake defaults to avoid excessive
    # console warnings (see issue #2120).
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
  endif()

  # Consumers who run into LNK4099 warnings can pass /Z7 instead (see issue #385).
  set(COMMON_DEBUG_INFO_FLAG "/Zi" CACHE STRING "Optional flag specifying specific /Z flag to use")

  # Consumers using different runtime types may want to pass different flags
  set(COMMON_RUNTIME_LIBRARY_FLAG "/MT" CACHE STRING "Optional flag specifying which runtime to use")
  if (COMMON_RUNTIME_LIBRARY_FLAG)
    list(APPEND COMMON_COMPILER_FLAGS_DEBUG ${COMMON_RUNTIME_LIBRARY_FLAG}d)
    list(APPEND COMMON_COMPILER_FLAGS_RELEASE ${COMMON_RUNTIME_LIBRARY_FLAG})
  endif()

  # Platform-specific compiler/linker flags.
  set(COMMON_LIBTYPE STATIC)
  list(APPEND COMMON_COMPILER_FLAGS
    /MP           # Multiprocess compilation
    /Gy           # Enable function-level linking
    #/GR-          # Disable run-time type information
    /W4           # Warning level 4
    /WX           # Treat warnings as errors
    /wd4100       # Ignore "unreferenced formal parameter" warning
    /wd4127       # Ignore "conditional expression is constant" warning
    /wd4244       # Ignore "conversion possible loss of data" warning
    /wd4481       # Ignore "nonstandard extension used: override" warning
    /wd4512       # Ignore "assignment operator could not be generated" warning
    /wd4701       # Ignore "potentially uninitialized local variable" warning
    /wd4702       # Ignore "unreachable code" warning
    /wd4996       # Ignore "function or variable may be unsafe" warning
    /wd4456       # Ignore "��������������һ����������" warning
    /wd4819       # Ignore "���ļ����������ڵ�ǰ����ҳ(936)�б�ʾ���ַ����뽫���ļ�����Ϊ Unicode ��ʽ�Է�ֹ���ݶ�ʧ" warning
    /wd4245       # Ignore "�з���/�޷��Ų�ƥ��" warning
    /wd4324       # Ignore "���ڶ���˵�������ṹ�����" warning
    ${COMMON_DEBUG_INFO_FLAG}
    )
  list(APPEND COMMON_COMPILER_FLAGS_DEBUG
    /RTC1         # Disable optimizations
    /Od           # Enable basic run-time checks
    )
  list(APPEND COMMON_COMPILER_FLAGS_RELEASE
    /O2           # Optimize for maximum speed
    /Ob2          # Inline any suitable function
    /GF           # Enable string pooling
    )
  list(APPEND COMMON_LINKER_FLAGS_DEBUG
    /DEBUG        # Generate debug information
    )
  list(APPEND COMMON_LINKER_FLAGS_RELEASE
    /DEBUG        # Release also Generate debug information
    )
  list(APPEND COMMON_EXE_LINKER_FLAGS
    /MANIFEST:NO        # No default manifest (see ADD_WINDOWS_MANIFEST macro usage)
    /LARGEADDRESSAWARE  # Allow 32-bit processes to access 3GB of RAM
    )
  list(APPEND COMMON_COMPILER_DEFINES
    WIN32 _WIN32 _WINDOWS             # Windows platform
    UNICODE _UNICODE                  # Unicode build
    WINVER=0x0601 _WIN32_WINNT=0x601  # Targeting Windows 7
    NOMINMAX                          # Use the standard's templated min/max
    WIN32_LEAN_AND_MEAN               # Exclude less common API declarations
    _HAS_EXCEPTIONS=0                 # Disable exceptions
    _ALLOW_COMPILER_AND_STL_VERSION_MISMATCH # Disable compiler version check, to avoid "cmake --build" possible cause compile error after vs update.
    )
  list(APPEND COMMON_COMPILER_DEFINES_DEBUG
    DEBUG _DEBUG                            # a debug build
    )
  list(APPEND COMMON_COMPILER_DEFINES_RELEASE
    NDEBUG _NDEBUG                    # Not a debug build
    )

  # Standard libraries.
  set(COMMON_STANDARD_LIBS
    comctl32.lib
    rpcrt4.lib
    shlwapi.lib
    ws2_32.lib
    )

  # Configure use of ATL.
  option(USE_ATL "Enable or disable use of ATL." ON)
  if(USE_ATL)
    # Locate the atlmfc directory if it exists. It may be at any depth inside
    # the VC directory. The cl.exe path returned by CMAKE_CXX_COMPILER may also
    # be at different depths depending on the toolchain version
    # (e.g. "VC/bin/cl.exe", "VC/bin/amd64_x86/cl.exe",
    # "VC/Tools/MSVC/14.10.25017/bin/HostX86/x86/cl.exe", etc).
    set(HAS_ATLMFC 0)
    get_filename_component(VC_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(VC_DIR_NAME ${VC_DIR} NAME)
    while(NOT ${VC_DIR_NAME} STREQUAL "VC")
      get_filename_component(VC_DIR ${VC_DIR} DIRECTORY)
      if(IS_DIRECTORY "${VC_DIR}/atlmfc")
        set(HAS_ATLMFC 1)
        break()
      endif()
      get_filename_component(VC_DIR_NAME ${VC_DIR} NAME)
    endwhile()

    # Determine if the Visual Studio install supports ATL.
    if(NOT HAS_ATLMFC)
      message(WARNING "ATL is not supported by your VC installation.")
      set(USE_ATL OFF)
    endif()
  endif()

  if(USE_ATL)
    list(APPEND COMMON_COMPILER_DEFINES
      USE_ATL   # Used by apps to test if ATL support is enabled
      )
  endif()
endif()

# windows/linux统一部署目录，项目的核心产出拷贝到部署目录，
# 调试时以部署目录为工作目录，安装包封包时从部署目录选取发布文件
set(COMMON_DEPLOYMENT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(COMMON_INTERMEDIATE_DIR "${CMAKE_BINARY_DIR}/global_intermediate")

# 旧直播姬工程的统一输出目录
if(PROJECT_ARCH STREQUAL "x86_64")
  # 64-bit architecture.
  set(BILILIVE_DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/build/x64/$<CONFIGURATION>")
elseif(PROJECT_ARCH STREQUAL "x86")
  # 32-bit architecture.
  set(BILILIVE_DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/build/Win32/$<CONFIGURATION>")
endif()
# "${CMAKE_BINARY_DIR}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/Plugins/org_test_plugins/"

# 中间目录，目录中放置的是项目生成时、编译时要引用到，然后生成、构建、编译过程中有可能会变化的文件
# 比如资源ID定义头文件、版本信息文件等
set(RES_DEPLOYMENT_DIR ${COMMON_INTERMEDIATE_DIR}/livehime)


## 将资源头文件拷贝到中间目录，
## 以便于后续的项目在cmake的时候能正确的找到磁盘上的资源ID定义.h文件
#message(STATUS "copy ${CMAKE_SOURCE_DIR}/resources/livehime/grit/*.h to: ${RES_DEPLOYMENT_DIR}/grit")
#set(GRIT_FILES
#  ${CMAKE_SOURCE_DIR}/resources/livehime/grit/generated_resources.h
#  ${CMAKE_SOURCE_DIR}/resources/livehime/grit/theme_resources.h
#  )
#foreach(fname ${GRIT_FILES})
#    file(COPY ${fname} DESTINATION ${RES_DEPLOYMENT_DIR}/grit)
#endforeach()


#if(OS_WINDOWS)
#    # Windows下把版本信息模板文件以正式版本信息的后缀（.rc）拷贝到中间目录，
#    # 以便于后续的项目在cmake的时候能正确的找到磁盘上的.rc文件
#    message(STATUS "copy ${CMAKE_SOURCE_DIR}/resources/livehime/version/win.rc.in to: ${RES_DEPLOYMENT_DIR}")
#
#    set(DEST_WINRC_FILES livehime.rc livehime_main_lib.rc livehime_secret_lib.rc)
#    set(SRC_WINRC_IN_FILE win.rc.in)
#    set(SRC_WINRC_FILE ${CMAKE_SOURCE_DIR}/resources/livehime/version/${SRC_WINRC_IN_FILE})
#    foreach(FILENAME ${DEST_WINRC_FILES})
#        #message(STATUS "copy ${SRC_WINRC_IN_FILE} to: ${RES_DEPLOYMENT_DIR}/${FILENAME}")
#        file(COPY ${SRC_WINRC_FILE} DESTINATION ${RES_DEPLOYMENT_DIR})
#        file(RENAME ${RES_DEPLOYMENT_DIR}/${SRC_WINRC_IN_FILE} ${RES_DEPLOYMENT_DIR}/${FILENAME})
#    endforeach()
#endif()
