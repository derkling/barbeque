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

set (BBQUE_PATH_BBQ     "sbin")
set (BBQUE_PATH_CONF    "etc/bbque")
set (BBQUE_PATH_RECIPES "etc/bbque/recipes")
set (BBQUE_PATH_HEADERS "usr/include/bbque")
set (BBQUE_PATH_RTLIB   "usr/lib/bbque")
set (BBQUE_PATH_PLUGINS "usr/lib/bbque/plugins")
set (BBQUE_PATH_DOCS    "usr/share/bbque")
set (BBQUE_PATH_VAR     "var/bbque")


################################################################################
# Common initialization settings
################################################################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake/")

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
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wextra -pedantic")
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
		ResourceAbstraction module" ON)

################################################################################
# Subdirectories analysis
################################################################################

# Recurse into project subfolders
add_subdirectory (rtlib)
add_subdirectory (plugins)
add_subdirectory (bbque)
add_subdirectory (testing)


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
message ( STATUS )
message ( STATUS "Default values could be changes at command line, with:")
message ( STATUS "  cmake -D<Variable>=<Value>" )
message ( STATUS "or use the '-i' cmake flag to run in interactive mode" )
message ( STATUS "======================================================"
                 "==========================" )
message(STATUS)

