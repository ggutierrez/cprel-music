# Detection of the CPrel constraint system.
# It assumes that if the system is installed so is gecode. No explicit check for
# gecode is made.

# Exports:
# CPREL_HDRS: Header files to be included
# CPREL_LIBS: Libraries
# CPREL_FOUND: boolean : whether cprel was found or not
set(CPREL_HDRS)
set(CPREL_LIBS)

# Find the domain representation library
#   - Headers
find_path(BDDDOMAIN_HDRS  bdddomain/grelation.hh)
# Look for the domain representation library
find_library(BDDDOMAIN_LIB NAMES bdddomain)


if (BDDDOMAIN_HDRS AND BDDDOMAIN_LIB)
  set(CPREL_FOUND Yes)
  list(APPEND CPREL_LIBS ${BDDDOMAIN_LIB})
  list(APPEND CPREL_HDRS ${BDDDOMAIN_HDRS})
  set(BDDDOMAIN_SUPPORT_LIBS cudd epd mtr obj st util)
  foreach(domain_support_lib ${BDDDOMAIN_SUPPORT_LIBS})
    find_library(SUPPORT_LIB_${domain_support_lib} NAMES ${domain_support_lib})
    list(APPEND CPREL_LIBS ${SUPPORT_LIB_${domain_support_lib}})
  endforeach()
else()
  message(FATAL_ERROR "The domain representation library was not found")
endif()

find_path(GECODE_CPREL_HDRS  cprel/cprel.hh)
# Look for the domain representation library
find_library(CPREL_LIB NAMES gecodecprel)

list(APPEND CPREL_LIBS ${CPREL_LIB})
list(APPEND CPREL_HDRS ${GECODE_CPREL_HDRS})

find_package(Gecode)
list(APPEND CPREL_LIBS ${Gecode_LIBRARIES})