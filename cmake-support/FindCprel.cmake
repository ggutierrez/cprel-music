# Detection of the CPrel constraint system.
# It assumes that if the system is installed so is gecode. No explicit check for
# gecode is made.

# Exports:
# CPREL_HDRS: Header files to be included
# CPREL_LIBS: Libraries

set(CPREL_HDRS)
set(CPREL_LIBS)

# Find the domain representation library
#   - Headers
find_path(BDDDOMAIN_HDRS  bdddomain/grelation.hh)
# Look for the library
find_library(BDDDOMAIN_LIB NAMES bdddomain)

if (BDDDOMAIN_HDRS AND BDDDOMAIN_LIB)
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
