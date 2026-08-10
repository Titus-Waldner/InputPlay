cmake_minimum_required(VERSION 3.20)

project(
    InputPlay
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    InputPlayCore
    STATIC
    src/CancellationSession.cpp
    src/DisplayMetadata.cpp
    src/DryRunBackend.cpp
    src/InputRecorder.cpp
    src/PlaybackEngine.cpp
    src/Recording.cpp
    src/RecordingFile.cpp
    src/RecordingValidator.cpp
    src/SendInputBackend.cpp
    src/Settings.cpp
	src/RecordingSummary.cpp
)

target_include_directories(
    InputPlayCore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
    InputPlay
    src/main.cpp
    src/CommandLine.cpp
)

target_link_libraries(
    InputPlay
    PRIVATE
    InputPlayCore
)

if(MINGW)
    target_link_options(
        InputPlay
        PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()