################################################################################
# The Barbeque RTRM Building Configuration
################################################################################

# Define CMake compatibility version
cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

# The name of this project.
project (BarbequeRTRM)

################################################################################
# Installation items
################################################################################

# Barbeque DAEMON configuraiton
set (BBQUE_DAEMON_NAME     "bbqued")
set (BBQUE_DAEMON_UID      "bbque")
set (BBQUE_DAEMON_GID      "rtrm")
set (BBQUE_DAEMON_LOCKFILE "var/lock/bbqued.lock")
set (BBQUE_DAEMON_RUNDIR   "")

# Barbeque installation pats
set (BBQUE_PATH_BBQ     "sbin")
set (BBQUE_PATH_CONF    "etc/bbque")
set (BBQUE_PATH_RECIPES "etc/bbque/recipes")
set (BBQUE_PATH_HEADERS "include/bbque")
set (BBQUE_PATH_RTLIB   "lib/bbque")
set (BBQUE_PATH_PLUGINS "lib/bbque/plugins")
set (BBQUE_PATH_DOCS    "usr/share/bbque")
set (BBQUE_PATH_VAR     "var/bbque")


################################################################################
# Common initialization settings
################################################################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake/")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/.cmake"
			DESTINATION "${BBQUE_PATH_RTLIB}"
			FILES_MATCHING PATTERN "*.cmake*")

# Make sure the compiler can find include files
include_directories (
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include
)

message ( STATUS )
message ( STATUS "Checking dependencies..." )

# Setup the source tree versioning number
include (GetGitRevisionDescription)
git_describe(GIT_SHA1)
set (VERSION "${GIT_SHA1}")

# Check if running un UNIX like systems
if (UNIX)
	add_definitions(-DUNIX)
endif(UNIX)


################################################################################
# Build version specific configurations
################################################################################
# Choose the type of build, options are:
#    None Debug Release RelWithDebInfo MinSizeRel

# Set default release version to RELEASE
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

# Options common to all build version
set (CMAKE_CXX_FLAGS "-std=c++0x -Wall")

# Options for build version: DEBUG
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wextra -pedantic -DBBQUE_DEBUG")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_DEBUG ON)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options for build version: DESKTOP RELEASE
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

# Options for build version: EMBEDDED RELEASE
set (CMAKE_CXX_FLAGS_MINSIZEREL "-Os")


################################################################################
# Installation locations
################################################################################

# set default install prefix to project root directory
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
	set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/out" )
endif (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")


################################################################################
# User selectable options
################################################################################

# RPC Channel abstraction: use FIFO implementation (defaulf: YES)
option (BBQUE_RPC_FIFO "Build RTLib with FIFO channel support" ON)

# Platform data: use hard-coded TEST configuration (defaulf: YES)
option (BBQUE_TEST_PLATFORM_DATA "Use TEST Platform Data instead of
		ResourceAbstraction module" OFF)

################################################################################
# Lookup for library dependencies
################################################################################

# Force usage of BOSP provided BOOST installation
set (Boost_NO_SYSTEM_PATHS ON)
set (BOOST_ROOT ${CMAKE_INSTALL_PREFIX})
set (BOOST_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})

# Check for required libs and packages (headers + lib)
find_package(Boost 1.45.0 REQUIRED system program_options filesystem)
find_package(CGroup REQUIRED)

# Add compilation dependencies
include_directories(
	${Boost_INCLUDE_DIRS}
	${CGroup_INCLUDE_DIRS}
)

# Add linking dependencies
link_directories(
	${Boost_LIBRARY_DIRS}
	${CGroup_LIBRARY_DIRS}
)

################################################################################
# Subdirectories analysis
################################################################################

# Recurse into project subfolders
add_subdirectory (rtlib)
add_subdirectory (plugins)
add_subdirectory (bbque)
add_subdirectory (test)


################################################################################
# Doxygen Documentation
################################################################################
# check if doxygen is even installed
find_package(Doxygen)
if (DOXYGEN_FOUND STREQUAL "NO")
	message(FATAL_ERROR "Doxygen not found. "
		"Please get a copy http://www.doxygen.org")
endif (DOXYGEN_FOUND STREQUAL "NO")
# prepare doxygen configuration file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# add doxygen as target
add_custom_target (doxygen ${DOXYGEN_EXECUTABLE}
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# cleanup $build/api-doc on "make clean"
set_property(DIRECTORY APPEND PROPERTY
		     ADDITIONAL_MAKE_CLEAN_FILES api-doc)
# add doxygen as dependency to doc-target
get_target_property (DOC_TARGET doc TYPE)
if (NOT DOC_TARGET)
	add_custom_target(doc)
endif ()
add_dependencies (doc doxygen)
# install HTML API documentation and manual pages
set (DOC_PATH "${BBQUE_PATH_DOCS}")
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api-doc/html
	DESTINATION ${DOC_PATH} OPTIONAL)
# install man pages into packages, scope is now project root..
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api-doc/man/man3
	DESTINATION ${DOC_PATH} OPTIONAL)


################################################################################
# Configuration resume
################################################################################

# display status message for important variables
message ( STATUS )
message ( STATUS "=====[ Barbeque RTRM - Building System Configuration ]"
                 "==========================" )
message ( STATUS "Barbeque Version...... ${VERSION}" )
message ( STATUS "Build type............ ${CMAKE_BUILD_TYPE}" )
message ( STATUS "Installation prefix... ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "   Barbeuqe RTRM...... <prefix>/${BBQUE_PATH_BBQ}" )
message ( STATUS "   Configuration...... <prefix>/${BBQUE_PATH_CONF}" )
message ( STATUS "   Recipes............ <prefix>/${BBQUE_PATH_RECIPES}" )
message ( STATUS "   Plugins............ <prefix>/${BBQUE_PATH_PLUGINS}" )
message ( STATUS "   Var(RPC-Channel)... <prefix>/${BBQUE_PATH_VAR}" )
message ( STATUS "   Headers............ <prefix>/${BBQUE_PATH_HEADERS}" )
message ( STATUS "   RTLib library...... <prefix>/${BBQUE_PATH_RTLIB}" )
message ( STATUS "   Documentation...... <prefix>/${BBQUE_PATH_DOCS}" )
message ( STATUS "Dependencies:")
message ( STATUS "   Boost library...... ${Boost_LIBRARY_DIRS}" )

message ( STATUS )
message ( STATUS "Default values could be changes at command line, with:")
message ( STATUS "  cmake -D<Variable>=<Value>" )
message ( STATUS "or use the '-i' cmake flag to run in interactive mode" )
message ( STATUS "======================================================"
                 "==========================" )
message(STATUS)

