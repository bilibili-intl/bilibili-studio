# copy from obs-studio\cmake\copy_on_debug_helper.cmake

string(REGEX REPLACE "\\.(dll|exe)$" ".pdb" FNAME "${FNAME}")

if(EXISTS "${INPUT}/${FNAME}")
	file(COPY "${INPUT}/${FNAME}" DESTINATION "${OUTPUT}")
endif()

#if(CONFIG STREQUAL Debug OR CONFIG STREQUAL RelWithDebInfo)
#	file(COPY "${INPUT}/${FNAME}" DESTINATION "${OUTPUT}")
#elseif(EXISTS "${OUTPUT}/${FNAME}")
#	file(REMOVE "${OUTPUT}/${FNAME}")
#endif()
