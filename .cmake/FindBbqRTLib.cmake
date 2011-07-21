#
# Locate Barbeque RTLib include paths and libraries

# This module defines
# BBQUE_RTLIB_INCLUDE_DIR, where to find rtlib.h, etc.
# BBQUE_RTLIB_LIBRARIES, the libraries to link against to use the RTLib.
# BBQUE_RTLIB_FOUND, If false, don't try to use RTLib.

find_path(BBQUE_RTLIB_INCLUDE_DIR bbque/rtlib.h)
find_library(BBQUE_RTLIB_LIBRARY bbque_rtlib
	PATH_SUFFIXES bbque)

set(BBQUE_RTLIB_FOUND 0)
if (BBQUE_RTLIB_INCLUDE_DIR)
  if (BBQUE_RTLIB_LIBRARY)
    set(BBQUE_RTLIB_FOUND 1)
    message(STATUS "Found RTLib: ${BBQUE_RTLIB_LIBRARY}")
  else (BBQUE_RTLIB_INCLUDE_DIR)
    message(FATAL_ERROR "Barbeque RTLib NOT FOUND!")
  endif (BBQUE_RTLIB_LIBRARY)
endif (BBQUE_RTLIB_INCLUDE_DIR)

mark_as_advanced(
  BBQUE_RTLIB_INCLUDE_DIR
  BBQUE_RTLIB_LIBRARY
)
