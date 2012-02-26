cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-support")

set(PROJECT_NAME "CPRelTools")
set(PROJECT_VERSION "0.9.0")
project(${PROJECT_NAME})
##########################################################################
# Compiler information
##########################################################################
find_package(Compiler)
##########################################################################
# Detection of the midi library
##########################################################################
find_package(Jdksmidi)
include_directories(${MIDI_HDR})
##########################################################################
# Library and utility creation
##########################################################################

include_directories(${CMAKE_SOURCE_DIR})

add_executable(midi-properties midi-properties.cpp)
target_link_libraries(midi-properties midi_lib)
