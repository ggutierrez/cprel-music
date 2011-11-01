cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "CPRelTools")
set(PROJECT_VERSION "0.9.0")
project(${PROJECT_NAME})
##########################################################################
# Command line options
##########################################################################
option(BUILD_EXAMPLES "Build examples " NO)
##########################################################################
# Compiler information
##########################################################################
if(CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    message(STATUS "GCC version ${GCC_VERSION}")
  else()
    message(FATAL_ERROR "GCC version 4.6 or greater is required")
  endif()
else()
  message(WARNING "Compiler: ${CMAKE_CXX_COMPILER_ID}")
  message(WARNING "${CMAKE_COMPILER_IS_CLANGCXX}")
endif()
##########################################################################
# System information
##########################################################################
message(STATUS "Building for architecture: ${CMAKE_SYSTEM}")
##########################################################################
# Additional compiler flags
##########################################################################
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++0x" C0X_SUPPORT)
if(C0X_SUPPORT)
  message(STATUS "CXX has c++0x support")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "c++0x capable compiler is needed to build this project at this time")
endif()

check_cxx_compiler_flag(-Wall FLAG_WALL)
if(FLAG_WALL)
  add_definitions(-Wall)
endif()

check_cxx_compiler_flag(-Wextra FLAG_WEXTRA)
if(FLAG_WEXTRA)
  add_definitions(-Wextra)
endif()

check_cxx_compiler_flag(-fdiagnostics-show-option FLAG_DIAGNOSTIC)
if(FLAG_DIAGNOSTIC)
  add_definitions(-fdiagnostics-show-option)
endif()
##########################################################################
# Detection of the domain representation library
##########################################################################
find_library(BDDDOMAIN bdddomain)
find_path(BDDDOMAIN_HDRS bdddomain/rel-impl.hh)
 
if(BDDDOMAIN_HDRS AND BDDDOMAIN)
  include_directories(${BDDDOMAIN_HDRS})
  include_directories(${BDDDOMAIN_HDRS}/cudd)
  include_directories(${BDDDOMAIN_HDRS}/mtr)
  include_directories(${BDDDOMAIN_HDRS}/epd)
  include_directories(${BDDDOMAIN_HDRS}/st)
  include_directories(${BDDDOMAIN_HDRS}/util)
  include_directories(${BDDDOMAIN_HDRS}/obj)
  find_library(CUDD cudd)
  list(APPEND BDDDOMAIN ${CUDD})
  find_library(CUDDOBJ cuddobj)
  list(APPEND BDDDOMAIN ${CUDDOBJ})
  find_library(CUDDEPD epd)
  list(APPEND BDDDOMAIN ${CUDDEPD})
  find_library(CUDDST st)
  list(APPEND BDDDOMAIN ${CUDDST})
  find_library(CUDDMPR mpr)
  list(APPEND BDDDOMAIN ${CUDDMPR})
  find_library(CUDDUTIL util)
  list(APPEND BDDDOMAIN ${CUDDUTIL})
  #message(STATUS "Domain library at: ${BDDDOMAIN}")
  #message(STATUS "Domain library at: ${BDDDOMAIN_HDRS}")
else()
  message(FATAL_ERROR "Domain representation library not found")
endif()
##########################################################################
# Detection of the midi library
##########################################################################
find_library(MIDILIB jdksmidi)
find_path(MIDILIB_HDRS jdksmidi/world.h)
if(MIDILIB_HDRS AND MIDILIB)
  include_directories(${MIDILIB_HDRS})
  message(STATUS "Midi library at: ${MIDILIB}")
else()
  message(FATAL_ERROR "Midi library not found, please download it with:")
  message(FATAL_ERROR "git clone https://github.com/jdkoftinoff/jdksmidi.git")
endif()
##########################################################################
# Library and utility creation
##########################################################################

include_directories(${CMAKE_SOURCE_DIR})

set(CSVREADER_SRCS
  tools/csvreader.cpp
  tools/csvreader.hh
)

add_library(cpreltools ${CSVREADER_SRCS})
target_link_libraries(cpreltools ${BDDDOMAIN})

add_executable(midistats midi.cpp)
target_link_libraries(midistats cpreltools ${BDDDOMAIN})

add_executable(midi-properties midi-properties.cpp)
target_link_libraries(midi-properties cpreltools ${BDDDOMAIN} ${MIDILIB})
