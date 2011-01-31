
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)


# Check for libs and packages (headers + lib)



find_package(Boost 1.40.0 REQUIRED system program_options filesystem)
find_package(Log4CPP)

# Remove Log4CPP dependencies (if not available)
if (LOG4CPP_FOUND EQUAL 0)
	set (log4cpp_INCLUDE_DIRS "")
	set (log4cpp_LIBRARY_DIRS "")
	set (LOG4CPP_LIBRARIES "")
	set (LOGGER_INSTANCE "console")
	message ("Log4CPP not found, using Console logger.")
else (LOG4CPP_FOUND EQUAL 0)
	set (LOGGER_INSTANCE "logger_log4cpp")
	set (BBQUE_HAVE_LOG4CPP ON)
endif (LOG4CPP_FOUND EQUAL 0)

# Add compilation dependencies
include_directories(
	${Boost_INCLUDE_DIRS}
	${log4cpp_INCLUDE_DIRS}
)

# Add linking dependencies
link_directories(
	${Boost_LIBRARY_DIRS}
	${log4cpp_LIBRARY_DIRS}
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -ldl")

# Generate configuration file
configure_file (
	"${PROJECT_SOURCE_DIR}/include/bbque/config.h.in"
	"${PROJECT_BINARY_DIR}/include/bbque/config.h"
)

# Add "barbeque" target binary
set (BARBEQUE_SRC main version object logger ${LOGGER_INSTANCE})
set (BARBEQUE_SRC dynamic_library plugin_manager ${BARBEQUE_SRC})
add_executable (barbeque ${BARBEQUE_SRC})

# Linking dependencies
target_link_libraries(
	barbeque
	${LOG4CPP_LIBRARIES}
	${Boost_LIBRARIES}
)

# Linking static plugins
target_link_libraries(
	barbeque
	-Wl,-whole-archive plugin_testingobject -Wl,-no-whole-archive
)

