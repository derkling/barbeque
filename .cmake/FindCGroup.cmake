#
# Locate Control Group include paths and libraries
# libcgroup can be found at http://libcg.sourceforge.net/

# This module defines
# CGroup_INCLUDE_DIR, where to find libcgroup.h, etc.
# CGroup_LIBRARIES, the libraries to link against to use libcgroup.
# CGroup_FOUND, If false, don't try to use libcgroup.

find_path(CGroup_INCLUDE_DIR libcgroup.h)
find_library(CGroup_LIBRARIES cgroup)

set(CGroup_FOUND 0)
if (CGroup_INCLUDE_DIR)
  if (CGroup_LIBRARIES)
    set(CGroup_FOUND 1)
    message(STATUS "Found CGroup: ${CGroup_LIBRARIES}")
  endif (CGroup_LIBRARIES)
endif (CGroup_INCLUDE_DIR)

mark_as_advanced(
  CGroup_INCLUDE_DIR
  CGroup_LIBRARIES
)
