string(REGEX REPLACE "\\.(dll|exe)$" ".pdb" FNAME "${FNAME}")

# modify [
# Both Debug and Release Generate debug information
#if(CONFIG STREQUAL Debug OR CONFIG STREQUAL RelWithDebInfo)
if(CONFIG STREQUAL Debug OR CONFIG STREQUAL Release OR CONFIG STREQUAL RelWithDebInfo)
# ]
	file(COPY "${INPUT}/${FNAME}" DESTINATION "${OUTPUT}")
elseif(EXISTS "${OUTPUT}/${FNAME}")
	file(REMOVE "${OUTPUT}/${FNAME}")
endif()
