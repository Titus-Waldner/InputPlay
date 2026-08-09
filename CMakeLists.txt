cmake_minimum_required(VERSION 3.20)

project(InputPlay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(
    InputPlay
    src/main.cpp
    src/Recording.cpp
    src/RecordingFile.cpp
    src/DryRunBackend.cpp
    src/SendInputBackend.cpp
    src/InputRecorder.cpp
    src/DisplayMetadata.cpp
    src/Settings.cpp
	src/RecordingValidator.cpp
	src/CancellationSession.cpp
)