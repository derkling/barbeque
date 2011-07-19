#
# Locate log4cpp include paths and libraries
# log4cpp can be found at http://log4cpp.sourceforge.net/
# Written by Manfred Ulz, manfred.ulz_at_tugraz.at

# This module defines
# LOG4CPP_INCLUDE_DIR, where to find ptlib.h, etc.
# LOG4CPP_LIBRARIES, the libraries to link against to use pwlib.
# LOG4CPP_FOUND, If false, don't try to use pwlib.

find_path(LOG4CPP_INCLUDE_DIR log4cpp/Category.hh)
find_library(LOG4CPP_LIBRARIES log4cpp)

set(LOG4CPP_FOUND 0)
if (LOG4CPP_INCLUDE_DIR)
  if (LOG4CPP_LIBRARIES)
    set(LOG4CPP_FOUND 1)
    message(STATUS "Found Log4CPP: ${LOG4CPP_LIBRARIES}")
  endif (LOG4CPP_LIBRARIES)
endif (LOG4CPP_INCLUDE_DIR)

mark_as_advanced(
  LOG4CPP_INCLUDE_DIR
  LOG4CPP_LIBRARIES
)
