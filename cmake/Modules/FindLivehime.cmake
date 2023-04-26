
# livehime directory paths.
set(LIVEHIME_SOURCE_DIR     "${CMAKE_SOURCE_DIR}/bililive")
set(LIVEHIME_THIRD_PARTY_DIR     "${CMAKE_SOURCE_DIR}/third_party")

# CEF directory paths.
if(PROJECT_ARCH STREQUAL "x86")
  set(CEF_SOURCE_DIR                  "${CMAKE_SOURCE_DIR}/cef/cef_core")
  set(CEF_RESOURCE_DIR                "${CEF_SOURCE_DIR}/Win32/Resources")
  set(CEF_BINARY_DIR                  "${CEF_SOURCE_DIR}/Win32/$<CONFIGURATION>")
  set(CEF_BINARY_DIR_DEBUG            "${CEF_SOURCE_DIR}/Win32/Debug")
  set(CEF_BINARY_DIR_RELEASE          "${CEF_SOURCE_DIR}/Win32/Release")
# ֮���԰�libcef_dll_wrapper�������ļ��ŵ�${CEF_SOURCE_DIR}/build/libcef_dll_wrapperĿ¼��
# �Ƿ���obs-browserֱ��Find��obs-studio\plugins\obs-browser\FindCEF.cmake���ǰ������·����libcef_dll_wrapper���
  set(CEF_WARPPER_BINARY_DIR          "${CEF_SOURCE_DIR}/Win32/build/libcef_dll_wrapper/$<CONFIGURATION>")
  set(CEF_WARPPER_BINARY_DIR_DEBUG    "${CEF_SOURCE_DIR}/Win32/build/libcef_dll_wrapper/Debug")
  set(CEF_WARPPER_BINARY_DIR_RELEASE  "${CEF_SOURCE_DIR}/Win32/build/libcef_dll_wrapper/Release")

else()
  set(CEF_SOURCE_DIR                  "${CMAKE_SOURCE_DIR}/cef/cef_core")
  set(CEF_RESOURCE_DIR                "${CEF_SOURCE_DIR}/x64/Resources")
  set(CEF_BINARY_DIR                  "${CEF_SOURCE_DIR}/x64/$<CONFIGURATION>")
  set(CEF_BINARY_DIR_DEBUG            "${CEF_SOURCE_DIR}/x64/Debug")
  set(CEF_BINARY_DIR_RELEASE          "${CEF_SOURCE_DIR}/x64/Release")
  set(CEF_WARPPER_BINARY_DIR          "${CEF_SOURCE_DIR}/x64/build/libcef_dll_wrapper/$<CONFIGURATION>")
  set(CEF_WARPPER_BINARY_DIR_DEBUG    "${CEF_SOURCE_DIR}/x64/build/libcef_dll_wrapper/Debug")
  set(CEF_WARPPER_BINARY_DIR_RELEASE  "${CEF_SOURCE_DIR}/x64/build/libcef_dll_wrapper/Release")

  
endif()

# CEF library paths.
if(OS_LINUX)
  set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.so")
  set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.so")

  # List of CEF binary files.
  set(CEF_BINARY_FILES
    chrome-sandbox
    libcef.so
    libEGL.so
    libGLESv2.so
    natives_blob.bin
    snapshot_blob.bin
    v8_context_snapshot.bin
    swiftshader
    )

  # List of CEF resource files.
  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
    )
elseif(OS_WINDOWS)
  set(CEF_LIB_DEBUG                 "${CEF_BINARY_DIR_DEBUG}/libcef.lib")
  set(CEF_LIB_RELEASE               "${CEF_BINARY_DIR_RELEASE}/libcef.lib")
  set(CEF_WARPPER_LIB_DEBUG         "${CEF_WARPPER_BINARY_DIR_DEBUG}/libcef_dll_wrapper.lib")
  set(CEF_WARPPER_LIB_RELEASE       "${CEF_WARPPER_BINARY_DIR_RELEASE}/libcef_dll_wrapper.lib")

  # List of CEF binary files.
  set(CEF_BINARY_FILES
    chrome_elf.dll
    d3dcompiler_47.dll
    libcef.dll
    libEGL.dll
    libGLESv2.dll
    natives_blob.bin
    snapshot_blob.bin
    v8_context_snapshot.bin
    swiftshader
    )

  # List of CEF resource files.
  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
    )
elseif(OS_MACOSX)
  set(CEF_LIB_DEBUG                 "${CEF_BINARY_DIR_DEBUG}/Chromium Embedded Framework.framework")
  set(CEF_LIB_RELEASE               "${CEF_BINARY_DIR_RELEASE}/Chromium Embedded Framework.framework")
  set(CEF_WARPPER_LIB_DEBUG         "${CEF_WARPPER_BINARY_DIR_DEBUG}/libcef_dll_wrapper.a")
  set(CEF_WARPPER_LIB_RELEASE       "${CEF_WARPPER_BINARY_DIR_RELEASE}/libcef_dll_wrapper.a")
endif()

if(NOT OS_MACOSX)
  set(CEF_LIB
      optimized ${CEF_LIB_RELEASE}
      debug ${CEF_LIB_DEBUG})
  set(CEF_WARPPER_LIB
      optimized ${CEF_WARPPER_LIB_RELEASE}
      debug ${CEF_WARPPER_LIB_DEBUG})
else()
  #set(CEF_LIB
  #  $<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>
  #  $<$<CONFIG:Release>:${CEF_LIB_RELEASE}>)
  set(CEF_LIB
    ${CEF_LIB_RELEASE})
  set(CEF_WARPPER_LIB
    $<$<CONFIG:Debug>:${CEF_WARPPER_LIB_DEBUG}>
    $<$<CONFIG:Release>:${CEF_WARPPER_LIB_RELEASE}>)
endif()

macro(ADD_CEF_INCLUDE_DIRECTORIES target)
  target_include_directories(${target} PRIVATE ${CEF_SOURCE_DIR})
endmacro()


# OBS
set(OBS_SOURCE_DIR          "${CMAKE_SOURCE_DIR}/obs/obs-studio")
set(OBS_MAINLIB_SOURCE_DIR  "${CMAKE_SOURCE_DIR}/obs/obs-studio/libobs")
set(OBS_PLUGINS_SOURCE_DIR  "${CMAKE_SOURCE_DIR}/obs/obs-studio/plugins")
# OBS deps
if(OS_WINDOWS)
  if(PROJECT_ARCH STREQUAL "x86")
    set(OBS_DEPENDENCIES_DIR    "${CMAKE_SOURCE_DIR}/obs/dependencies2019/win32")
  else()
    set(OBS_DEPENDENCIES_DIR    "${CMAKE_SOURCE_DIR}/obs/dependencies2019/win64")
  endif()
elseif(OS_MACOSX)
  set(OBS_DEPENDENCIES_DIR    "${CMAKE_SOURCE_DIR}/obs/obsdeps")
endif()

macro(ADD_OBS_INCLUDE_DIRECTORIES target)
  target_include_directories(${target} PRIVATE ${OBS_SOURCE_DIR})
  target_include_directories(${target} PRIVATE ${OBS_MAINLIB_SOURCE_DIR})
  target_include_directories(${target} PRIVATE ${OBS_PLUGINS_SOURCE_DIR})
endmacro()


# thirdparty
set(LIVEHIME_THIRDPARTY_DIR     "${CMAKE_SOURCE_DIR}/third_party")
# sensetime
set(LIVEHIME_SENSETIME_DIR      "${LIVEHIME_THIRDPARTY_DIR}/sensetime-sdk")
set(LIVEHIME_SENSETIME_INCLUDE_DIR
  "${LIVEHIME_SENSETIME_DIR}"
  "${LIVEHIME_SENSETIME_DIR}/include"
  "${LIVEHIME_SENSETIME_DIR}/external"
  "${LIVEHIME_SENSETIME_DIR}/external/include"
  "${LIVEHIME_SENSETIME_DIR}/external/include/GL"
  "${LIVEHIME_SENSETIME_DIR}/external/include/GLFW"
  "${LIVEHIME_SENSETIME_DIR}/external/include/KHR"
  )
# meishe
set(LIVEHIME_MEISHE_DIR      "${LIVEHIME_THIRDPARTY_DIR}/meishe")
set(LIVEHIME_MEISHE_INCLUDE_DIR
  "${LIVEHIME_MEISHE_DIR}"
  "${LIVEHIME_MEISHE_DIR}/include"
  "${LIVEHIME_MEISHE_DIR}/include/sdkcore"
  "${LIVEHIME_MEISHE_DIR}/include/visioncore"
  "${LIVEHIME_MEISHE_DIR}/include/framework"
  "${LIVEHIME_MEISHE_DIR}/include/core"
  )
# boost
set(LIVEHIME_BOOST_DIR          "${LIVEHIME_THIRDPARTY_DIR}/boost_lite")
# openssl, used by obs-win-dshow
set(LIVEHIME_OPENSSL_DIR        "${LIVEHIME_THIRDPARTY_DIR}/openssl")
# openssl, used by obs-bvc-srt-stream
set(LIVEHIME_OPENSSL_102_DIR    "${LIVEHIME_THIRDPARTY_DIR}/openssl-1.0.2h")
# bvc-srt
set(LIVEHIME_BVC_SRT_DIR        "${LIVEHIME_THIRDPARTY_DIR}/bvc-srt-network")
# agora
set(LIVEHIME_AGORA_DIR          "${LIVEHIME_THIRDPARTY_DIR}/agora")
# webrtc
set(LIVEHIME_WEBRTC_DIR         "${LIVEHIME_THIRDPARTY_DIR}/webrtc")