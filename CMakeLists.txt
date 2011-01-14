
# The name of this project is "BARBEQUE".
project (BARBEQUE)

cmake_minimum_required (VERSION 2.6)
if (UNIX)
	add_definitions(-DUNIX)
endif(UNIX)


# Load additional CMake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake/")


# Make sure the compiler can find include files
include_directories (
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include
)

# Select default release version
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# Configure the building options for each release
# Common
set (CMAKE_CXX_FLAGS "-Wall")
# DEBUGGING
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wextra -pedantic")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_DEBUG ON)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
# DESKTOP RELEASE
set (CMAKE_CXX_FLAGS_RELEASE "-O3")
# EMBEDDED RELEASE
set (CMAKE_CXX_FLAGS_MINSIZEREL "-Os")



# Defines documentation files
set (DOC_FILES
	AUTHORS NEWS README)
set (DOC_PATH
	"share/doc/barbeque")
install (FILES ${DOC_FILES}
	DESTINATION ${DOC_PATH})

# Setup the source tree versioning number
include(GetGitRevisionDescription)
git_describe(GIT_SHA1)

# Setup packaging
set (CPACK_PACKAGE_DESCRIPTION
	"The Barbeque RTRM")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY
	"The RTRM protorype implementation for 2PARMA EU FP7 project")
set (CPACK_PACKAGE_NAME "barbeue")
set (CPACK_DEBIAN_PACKAGE_DEPENDS
	"libc6 (>= 2.11), libgcc1 (>= 1:4.1), liblog4cpp (>= 1.0)")

set (CPACK_PACKAGE_CONTACT "Patrick Bellasi <derkling@gmail.com>")
set (CPACK_PACKAGE_VENDOR "Politecnico di Milano")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "${GIT_SHA1}")
set (VERSION "${GIT_SHA1}")

set (CPACK_GENERATOR "DEB")
set (CPACK_PACKAGE_FILE_NAME
	"${CPACK_PACKAGE_NAME}-${VERSION}")
include (CPack)

# Check if doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND STREQUAL "NO")
	message (WARNING "Doxygen not found!")
	message ("To build documentation, please get a copy from http://www.doxygen.org")
else (DOXYGEN_FOUND STREQUAL "NO")
	include (BuildDoxygenDocumentation)
endif (DOXYGEN_FOUND STREQUAL "NO")

# Recurse into project subfolders
add_subdirectory (bbque)

