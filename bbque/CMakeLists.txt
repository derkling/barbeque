
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)

# Check for required libs and packages (headers + lib)
find_package(Boost 1.40.0 REQUIRED system program_options filesystem)

# Add compilation dependencies
include_directories(
	${Boost_INCLUDE_DIRS}
)

# Add linking dependencies
link_directories(
	${Boost_LIBRARY_DIRS}
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -ldl")
# This flags should be used only when a monolitic build profile will be defined
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

# Generate configuration file
configure_file (
	"${PROJECT_SOURCE_DIR}/include/bbque/config.h.in"
	"${PROJECT_BINARY_DIR}/include/bbque/config.h"
)

# Build subdirs
add_subdirectory(utils)
add_subdirectory(app)
add_subdirectory(res)

# Add "barbeque" target binary
set (BARBEQUE_SRC barbeque version object configuration_manager console_logger)
set (BARBEQUE_SRC platform_services dynamic_library ${BARBEQUE_SRC})
set (BARBEQUE_SRC plugin_manager modules_factory ${BARBEQUE_SRC})
set (BARBEQUE_SRC resource_manager application_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC application_proxy rpc_proxy ${BARBEQUE_SRC})
set (BARBEQUE_SRC signals_manager resource_scheduler ${BARBEQUE_SRC})
if(BBQUE_TEST_PLATFORM_DATA)
	# This is just a temporary replacement for the still missing
	# ResourceAbstraction module
	set (BARBEQUE_SRC test_platform_data ${BARBEQUE_SRC})
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_TEST_PLATFORM_DATA")
endif(BBQUE_TEST_PLATFORM_DATA)

add_executable (barbeque ${BARBEQUE_SRC})

# Linking dependencies
target_link_libraries(
	barbeque
	bbque_utils
	application_objs
	resource_accounting
	${Boost_LIBRARIES}
)

# Linking static plugins
target_link_libraries(
	barbeque
#	-Wl,-whole-archive plugin_test_dummy -Wl,-no-whole-archive
	-Wl,-whole-archive plugin_logger_log4cpp -Wl,-no-whole-archive
	-Wl,-whole-archive plugin_recipe_loader_xml -Wl,-no-whole-archive
)

