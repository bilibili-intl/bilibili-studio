project(win-decklink)

if(DISABLE_DECKLINK)
	message(STATUS "decklink plugin disabled")
	return()
endif()

include(IDLFileHelper)

# modify [
#include_directories(${CMAKE_SOURCE_DIR}/deps/libcaption)
include_directories(${OBS_STUDIO_SOURCE_DIR}/deps/libcaption)
# ]

set(win-decklink-sdk_IDLS
	decklink-sdk/DeckLinkAPI.idl
	)

set(win-decklink-sdk_HEADERS
	decklink-sdk/DeckLinkAPIVersion.h
	)

set(win-decklink_HEADERS
	../decklink-devices.hpp
	../DecklinkOutput.hpp
	../const.h
	../platform.hpp
	../DecklinkInput.hpp
	../DecklinkBase.h
	../decklink-device-instance.hpp
	../decklink-device-discovery.hpp
	../decklink-device.hpp
	../decklink-device-mode.hpp
	../audio-repack.h
	../audio-repack.hpp
	../util.hpp
	../OBSVideoFrame.h
	)

set(MODULE_DESCRIPTION "OBS DeckLink Windows module")
# modify [
#configure_file(${CMAKE_SOURCE_DIR}/cmake/winrc/obs-module.rc.in win-decklink.rc)
configure_file(${OBS_STUDIO_SOURCE_DIR}/cmake/winrc/obs-module.rc.in win-decklink.rc)
# ]
set(win-decklink_SOURCES
	../plugin-main.cpp
	../decklink-devices.cpp
	../DecklinkOutput.cpp
	../decklink-source.cpp
	../decklink-output.cpp
	../DecklinkInput.cpp
	../DecklinkBase.cpp
	../decklink-device-instance.cpp
	../decklink-device-discovery.cpp
	../decklink-device.cpp
	../decklink-device-mode.cpp
	../audio-repack.c
	platform.cpp
	../util.cpp
	win-decklink.rc
	../OBSVideoFrame.cpp)

add_idl_files(win-decklink-sdk_GENERATED_FILES
	${win-decklink-sdk_IDLS}
	)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	# modify [
	#"${CMAKE_SOURCE_DIR}/UI/obs-frontend-api"
	"${OBS_STUDIO_SOURCE_DIR}/UI/obs-frontend-api"
	# ]
)

add_library(win-decklink MODULE
	${win-decklink_SOURCES}
	${win-decklink_HEADERS}
	${win-decklink-sdk_HEADERS}
	${win-decklink-sdk_GENERATED_FILES}
	)

target_link_libraries(win-decklink
	libobs
	obs-frontend-api
	caption)
set_target_properties(win-decklink PROPERTIES FOLDER "plugins/decklink")

install_obs_plugin_with_data(win-decklink ../data)
