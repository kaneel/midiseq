cmake_minimum_required(VERSION 3.20)
project(MIDISequencer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    dearimgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG         dbb5eeaadffb6a3ba6a60de1290312e5802dba5a
)
FetchContent_MakeAvailable(dearimgui)

FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG         1e5b49925aa60065db52de44c366d446a902547b
)
FetchContent_MakeAvailable(rtmidi)

if(APPLE)
    execute_process(COMMAND brew --prefix sdl3 OUTPUT_VARIABLE SDL3_BREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(EXISTS "${SDL3_BREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${SDL3_BREW_PREFIX}")
        list(APPEND CMAKE_MODULE_PATH "${SDL3_BREW_PREFIX}/lib/cmake/SDL3")
        set(SDL3_DIR "${SDL3_BREW_PREFIX}/lib/cmake/SDL3")
        set(PACKAGE_PREFIX_DIR "${SDL3_BREW_PREFIX}" CACHE PATH "Path to SDL3 installation")
    endif()
endif()

# Find SDL3
find_package(SDL3 CONFIG REQUIRED)

# Add Dear ImGui source files directly
file(GLOB IMGUI_SRC
    ${dearimgui_SOURCE_DIR}/imgui*.cpp
    ${dearimgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
    ${dearimgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

SET(TARGET_H
    src/event_bus.h
    src/midi.h
    src/note.h
    src/playback.h
    src/sequence.h
    src/sequencer.h
    src/chain.h
)

SET(TARGET_SRC
    src/event_bus.cpp
    src/midi.cpp
    src/playback.cpp
    src/sequence.cpp
    src/sequencer.cpp
    src/chain.cpp
)

# Project targets and properties
add_executable(${PROJECT_NAME} src/main.cpp ${TARGET_H} ${TARGET_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE rtmidi SDL3::SDL3)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
)

