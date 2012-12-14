################################################################################
# The Barbeque RTRM Building Configuration
################################################################################

# Define CMake compatibility version
cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

# The name of this project.
project (BarbequeRTRM)


INCLUDE (${CONFIGPATH})

################################################################################
# Installation items
################################################################################

# Barbeque DAEMON configuraiton
set (BBQUE_DAEMON_NAME     "bbqued")
set (BBQUE_DAEMON_UID      "root")
set (BBQUE_DAEMON_LOCKFILE "var/lock/bbqued.lock")
set (BBQUE_DAEMON_PIDFILE  "var/run/bbqued.pid")
set (BBQUE_DAEMON_LOGFILE  "var/bbque.log")
set (BBQUE_DAEMON_RUNDIR   "")

# Barbeque RPC configuration
set (BBQUE_RPC_TIMEOUT     1000)

# Barbeque installation paths
set (BBQUE_PATH_BBQ        "sbin")
set (BBQUE_PATH_INIT       "etc/init.d")
set (BBQUE_PATH_DEFAULTS   "etc/default")
set (BBQUE_PATH_CONF       "etc/bbque")
set (BBQUE_PATH_PILS       "etc/bbque/pil")
set (BBQUE_PATH_RECIPES    "etc/bbque/recipes")
set (BBQUE_PATH_HEADERS    "include/bbque")
set (BBQUE_PATH_RTLIB      "lib/bbque")
set (BBQUE_PATH_PLUGINS    "lib/bbque/plugins")
set (BBQUE_PATH_APPS       "usr/bin")
set (BBQUE_PATH_DOCS       "usr/share/bbque")
set (BBQUE_PATH_CMAKE      "usr/share/bbque")
set (BBQUE_PATH_VAR        "var/bbque")

# Barbeque configuration file
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_CONF_FILE "bbque.conf_dbg")
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_CONF_FILE "bbque.conf")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Setup the CGroup-based Platform Integration Layers (PIL)
set (BBQUE_PIL_CGROUP_MOUNT "mnt/cgroup")

# Setup the RPATH and RWPATH
if ("${CONFIG_BOSP_RUNTIME_PATH}" STREQUAL "")
	set (CONFIG_BOSP_RUNTIME_PATH "${PROJECT_SOURCE_DIR}/../out")
endif ("${CONFIG_BOSP_RUNTIME_PATH}" STREQUAL "")
if ("${CONFIG_BOSP_RUNTIME_RWPATH}" STREQUAL "")
	set (CONFIG_BOSP_RUNTIME_RWPATH "${PROJECT_SOURCE_DIR}/../out/var/bbque")
endif ("${CONFIG_BOSP_RUNTIME_RWPATH}" STREQUAL "")

SET(CMAKE_INSTALL_RPATH "${CONFIG_BOSP_RUNTIME_PATH}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

################################################################################
# Common initialization settings
################################################################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake/")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/.cmake"
			DESTINATION "${BBQUE_PATH_CMAKE}"
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

# Add flags common to all builds
set (BOSP_CPP11 "0x")
if (GCC_TAG GREATER "46")
	set (BOSP_CPP11 "11")
endif (GCC_TAG GREATER "46")
set (CMAKE_CXX_FLAGS "-Wall -std=c++${BOSP_CPP11} -std=gnu++${BOSP_CPP11}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGCC_VERSION=${GCC_VERSION}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGCC_TAG=${GCC_TAG}")


# Add "Generic-Linux" specific flags
if (CONFIG_TARGET_LINUX)
set (BBQUE_TARGET_OS "Generic-Linux")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif (CONFIG_TARGET_LINUX)

# Add "Android-Linux" specific flags
if (CONFIG_TARGET_ANDROID)
set (BBQUE_TARGET_OS "Android-Linux")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I../../../out/include -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -D__STDINT_LIMITS -D_GLIBCXX_USE_C99_STDINT_TR1 -D__BIONIC__ -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fexceptions -frtti -mthumb -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -DANDROID  -Wa,--noexecstack")
# -ffunction-sections -fdata-sections -Wl,--gc-sections - REMOVED becaus of missing symbols
endif (CONFIG_TARGET_ANDROID)

# Check target bitness
set (BBQUE_TARGET_ARCH 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (BBQUE_TARGET_ARCH 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (BBQUE_BUILD_FLAVOR
	"${BBQUE_TARGET_OS} (${BBQUE_TARGET_ARCH}bit - ${CMAKE_BUILD_TYPE})")

# Options for build version: DEBUG
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wextra -pedantic -DBBQUE_DEBUG")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_DEBUG ON)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options for build version: DESKTOP RELEASE
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Options for build version: EMBEDDED RELEASE
set (CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")


################################################################################
# Installation locations
################################################################################

# set default install prefix to project root directory
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
	set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/out" )
endif (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")



################################################################################
# Lookup for library dependencies
################################################################################

# Force usage of BOSP provided BOOST installation
set (Boost_NO_SYSTEM_PATHS ON)
set (BOOST_ROOT ${CMAKE_INSTALL_PREFIX})
set (BOOST_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})

# Check for required libs and packages (headers + lib)
find_package(Boost 1.45.0 REQUIRED system program_options filesystem)
if (CONFIG_EXTERNAL_LIBCG)
	find_package(CGroup REQUIRED)
endif (CONFIG_EXTERNAL_LIBCG)

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
add_subdirectory (tools)
add_subdirectory (demos)
if (CONFIG_BBQUE_BUILD_TESTS)
add_subdirectory (test)
endif (CONFIG_BBQUE_BUILD_TESTS)



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
message ( STATUS "Build Flavor.......... ${BBQUE_BUILD_FLAVOR}" )
message ( STATUS "Installation prefix... ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "   Barbeuqe RTRM...... <prefix>/${BBQUE_PATH_BBQ}" )
message ( STATUS "   Configuration...... <prefix>/${BBQUE_PATH_CONF}/${BBQUE_CONF_FILE}" )
message ( STATUS "   Recipes............ <prefix>/${BBQUE_PATH_RECIPES}" )
message ( STATUS "   Plugins............ <prefix>/${BBQUE_PATH_PLUGINS}" )
message ( STATUS "   Var(RPC-Channel)... ${CONFIG_BOSP_RUNTIME_RWPATH}" )
message ( STATUS "   Headers............ <prefix>/${BBQUE_PATH_HEADERS}" )
message ( STATUS "   RTLib library...... <prefix>/${BBQUE_PATH_RTLIB}" )
message ( STATUS "   Documentation...... <prefix>/${BBQUE_PATH_DOCS}" )
message ( STATUS "Dependencies:")
message ( STATUS "   Boost library...... ${Boost_LIBRARY_DIRS}" )
message ( STATUS )
message ( STATUS "To change build settings, run the configuration menu with:")
message ( STATUS "   make menuconfig")
message ( STATUS "======================================================"
                 "==========================" )
message(STATUS)

