
# livehime directory paths.
set(LIVEHIME_WEBRTC_DIR     "${CMAKE_SOURCE_DIR}/third_party/webrtc")


file(GLOB_RECURSE source_files 
  LIST_DIRECTORIES false
  #RELATIVE ${source_dir}
  "${LIVEHIME_WEBRTC_DIR}/*.lib"
  )
#message(STATUS "source_files: ${source_files}")

set(WEBRTC_LIBRARY_COMMON "")
set(WEBRTC_LIBRARY_DEBUG "")
set(WEBRTC_LIBRARY_RELEASE "")

foreach(source_file ${source_files})
  string(REGEX MATCH "(.*)[Dd]ebug(.*)\\.lib" df ${source_file})
  if(df)
    #message(STATUS "Debug:  ${df}")
    list(APPEND WEBRTC_LIBRARY_DEBUG
      ${df}
      )
  endif()

  string(REGEX MATCH "(.*)[Rr]elease(.*)\\.lib" rf ${source_file})
  if(rf)
    #message(STATUS "Release:  ${rf}")
    list(APPEND WEBRTC_LIBRARY_RELEASE
      ${rf}
      )
  endif()

  if(NOT df AND NOT rf)
    #message(STATUS "Common:  ${source_file}")
    list(APPEND WEBRTC_LIBRARY_COMMON
      ${source_file}
      )
  endif()
endforeach()

list(APPEND WEBRTC_LIBRARY_DEBUG
  ${WEBRTC_LIBRARY_COMMON}
  )
list(APPEND WEBRTC_LIBRARY_RELEASE
  ${WEBRTC_LIBRARY_COMMON}
  )
#message(STATUS "WEBRTC_LIBRARY_COMMON:  ${WEBRTC_LIBRARY_COMMON}")
#message(STATUS "WEBRTC_LIBRARY_RELEASE:  ${WEBRTC_LIBRARY_RELEASE}")
#message(STATUS "WEBRTC_LIBRARY_DEBUG:  ${WEBRTC_LIBRARY_DEBUG}")