project(win-test)

# modify [
#include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/libobs")
include_directories(SYSTEM "${OBS_STUDIO_SOURCE_DIR}/libobs")
# ]

set(win-text_SOURCES
	test.cpp)

add_executable(win-test WIN32
	${win-text_SOURCES})
target_link_libraries(win-test
	libobs)
set_target_properties(win-test PROPERTIES FOLDER "tests and examples")
define_graphic_modules(win-test)
