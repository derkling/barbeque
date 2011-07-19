#
# Locate TiCPP include paths and libraries
#  TiCPP is a C++ wrapper for library TinyXML

# This module defines
# TICPP_INCLUDE_DIR, where to find ptlib.h, etc.
# TICPP_LIBRARIES, the libraries to link against to use pwlib.
# TICPP_FOUND, If false, don't try to use pwlib.

find_path(TICPP_INCLUDE_DIR ticpp.h)
find_library(TICPP_LIBRARIES ticpp)

set(TICPP_FOUND 0)
if (TICPP_INCLUDE_DIR)
  if (TICPP_LIBRARIES)
    set(TICPP_FOUND 1)
    message(STATUS "Found TiCPP: ${TICPP_LIBRARIES}")
  endif (TICPP_LIBRARIES)
endif (TICPP_INCLUDE_DIR)

mark_as_advanced(
  TICPP_INCLUDE_DIR
  TICPP_LIBRARIES
)
