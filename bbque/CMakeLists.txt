
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
# This flags should be used only when a monolitic build profile will be defined
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

# Generate configuration file
configure_file (
	"${PROJECT_SOURCE_DIR}/include/bbque/config.h.in"
	"${PROJECT_BINARY_DIR}/include/bbque/config.h"
)

# Setup configuration files based on installation paths
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bbque.conf.in"
	"${PROJECT_BINARY_DIR}/config/bbque.conf"
)
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bbque.conf_dbg.in"
	"${PROJECT_BINARY_DIR}/config/bbque.conf_dbg"
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
set (BARBEQUE_SRC platform_proxy ${BARBEQUE_SRC})
set (BARBEQUE_SRC pp/linux ${BARBEQUE_SRC})
set (BARBEQUE_SRC signals_manager scheduler_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC synchronization_manager ${BARBEQUE_SRC})
if(BBQUE_TEST_PLATFORM_DATA)
	# This is just a temporary replacement for the still missing
	# ResourceAbstraction module
	set (BARBEQUE_SRC test_platform_data ${BARBEQUE_SRC})
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_TEST_PLATFORM_DATA")
endif(BBQUE_TEST_PLATFORM_DATA)

# This should be converted to a KConfig option
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_PLATFORM_LINUX")

add_executable (barbeque ${BARBEQUE_SRC})

# Linking dependencies
target_link_libraries(
	barbeque
	bbque_utils
	resource_accounting
	apps
	${Boost_LIBRARIES}
	${CGroup_LIBRARIES}
)

# Linking static plugins
target_link_libraries(
	barbeque
	-Wl,-whole-archive bbque_logger_log4cpp -Wl,-no-whole-archive
	-Wl,-whole-archive bbque_recipe_loader_xml -Wl,-no-whole-archive
	-ldl -lrt
)

# Use link path ad RPATH
set_property(TARGET barbeque PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

# Install the Barbeque target
install (TARGETS barbeque RUNTIME
		DESTINATION ${BBQUE_PATH_BBQ}
		COMPONENT BarbequeRTRM)
install (FILES "${PROJECT_BINARY_DIR}/config/bbque.conf"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeRTRM)
install (FILES "${PROJECT_BINARY_DIR}/config/bbque.conf_dbg"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeRTRM)

