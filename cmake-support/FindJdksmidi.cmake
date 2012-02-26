##########################################################################
# Build the MIDI library
##########################################################################
# Creates the target midi_lib with the result of building jdksmidi.
# Exports the variables: 
# MIDI_HDR   : header files

##########################################################################
# Find Cudd library
##########################################################################
include(ExternalProject)
ExternalProject_Add(jdksmidi
  GIT_REPOSITORY git://github.com/jdkoftinoff/jdksmidi.git
  GIT_TAG master
  PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/use-clang.patch
  CONFIGURE_COMMAND ./configure
  BUILD_IN_SOURCE Yes
  BUILD_COMMAND make
  INSTALL_COMMAND echo "doing nothing..."
)

ExternalProject_Get_Property(jdksmidi source_dir)
#message(STATUS "The source of the midi library: ${source_dir}")

# Resource library for libjdksmidi.a
add_library(midi_lib UNKNOWN IMPORTED)
set_property(TARGET midi_lib PROPERTY IMPORTED_LOCATION ${source_dir}/tmp-target/build/lib/libjdksmidi.a)
add_dependencies(midi_lib jdksmidi)
set(MIDI_HDR ${source_dir}/include)