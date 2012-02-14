#
# Locate Barbeque Monitors include paths and libraries

# This module defines
# BBQUE_MONITORS_INCLUDE_DIR, where to find rtlib.h, etc.
# BBQUE_MONITORS_LIBRARIES, the libraries to link against to use the RTLib.
# BBQUE_MONITORS_FOUND, If false, don't try to use RTLib.

find_path(BBQUE_MONITORS_INCLUDE_DIR bbque/monitors/monitor.h)
find_library(BBQUE_MONITORS_LIBRARY bbque_monitors
	PATH_SUFFIXES bbque)

set(BBQUE_MONITORS_FOUND 0)
if (BBQUE_MONITORS_INCLUDE_DIR)
  if (BBQUE_MONITORS_LIBRARY)
    set(BBQUE_MONITORS_FOUND 1)
    message(STATUS "Found Monitors: ${BBQUE_MONITORS_LIBRARY}")
  else (BBQUE_RTLIB_INCLUDE_DIR)
    message(FATAL_ERROR "Barbeque Monitors NOT FOUND!")
  endif (BBQUE_MONITORS_LIBRARY)
endif (BBQUE_MONITORS_INCLUDE_DIR)

mark_as_advanced(
  BBQUE_MONITORS_INCLUDE_DIR
  BBQUE_MONITORS_LIBRARY
)
