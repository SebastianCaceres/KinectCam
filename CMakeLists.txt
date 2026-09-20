# CMakeList.txt : CMake project for KinectInfraredCam, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Point CMake to the custom modules

project (KinectInfraredCam VERSION 0.0.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(Kinect 1.8 REQUIRED)
find_package(DirectShow REQUIRED)

set(KinectInfraredCam_HEADERS
	KinectInfraredCam.h
	KinectVirtualCamera.h "util.h")

set(KinectInfraredCam_SOURCES
    KinectInfraredCam.cpp
	KinectInfraredCam.def # Module definition for exports
	KinectVirtualCamera.cpp
	Dll.cpp)

# Add source to this project's executable.

message("Found base classes ${DIRECTSHOW_BASECLASS_DIR}")
add_library(KinectInfraredCam MODULE ${KinectInfraredCam_HEADERS} ${KinectInfraredCam_SOURCES})
target_include_directories(KinectInfraredCam SYSTEM PRIVATE ${KINECT_INCLUDE_DIR} ${DIRECTSHOW_INCLUDE_DIRS})
target_link_libraries(KinectInfraredCam ${KINECT_LIBRARIES} ${DIRECTSHOW_STRMIIDS_LIBRARY} legacy_stdio_definitions.lib)
target_link_libraries(KinectInfraredCam winmm debug ${DIRECTSHOW_BASECLASS_DIR}/strmbasd.lib optimized strmbase)
set_target_properties(KinectInfraredCam PROPERTIES SUFFIX .ax COMPILE_FLAGS "/Zc:wchar_t-")

# Simple scripts to enable/disable filter registration with auto-elevation
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Reg.cmd" "@echo off\r\nnet session >nul 2>&1\r\nif %errorlevel% neq 0 (\r\n    echo Administrator permissions required. Requesting elevation...\r\n    powershell -Command \"Start-Process cmd -ArgumentList '/c \\\"\\\"%~f0\\\"\\\"' -Verb RunAs\"\r\n    exit /b\r\n)\r\n\"%WINDIR%\\System32\\regsvr32.exe\" \"%~dp0KinectInfraredCam.ax\"\r\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/UnReg.cmd" "@echo off\r\nnet session >nul 2>&1\r\nif %errorlevel% neq 0 (\r\n    echo Administrator permissions required. Requesting elevation...\r\n    powershell -Command \"Start-Process cmd -ArgumentList '/c \\\"\\\"%~f0\\\"\\\"' -Verb RunAs\"\r\n    exit /b\r\n)\r\n\"%WINDIR%\\System32\\regsvr32.exe\" /u \"%~dp0KinectInfraredCam.ax\"\r\n")

# Specify files to be installed
install(TARGETS KinectInfraredCam LIBRARY DESTINATION .)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Reg.cmd"
    "${CMAKE_CURRENT_BINARY_DIR}/UnReg.cmd"
    DESTINATION .)

include(CPack)
