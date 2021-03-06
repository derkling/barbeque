
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)

# Add "barbeque" specific flags
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
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
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bbque_2parma_demo2.conf.in"
	"${PROJECT_BINARY_DIR}/config/bbque_2parma_demo2.conf"
)

# Setup the daemon initialization script
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bbqued.in"
	"${PROJECT_BINARY_DIR}/config/bbqued"
	@ONLY
)
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bbqued.defaults.in"
	"${PROJECT_BINARY_DIR}/config/default/bbqued"
)

# Setup BOSP shell
configure_file (
	"${PROJECT_SOURCE_DIR}/config/bosp_init.env.in"
	"${PROJECT_BINARY_DIR}/config/bosp_init.env"
	@ONLY
)


# Build subdirs
add_subdirectory(utils)
add_subdirectory(app)
add_subdirectory(res)
if (NOT CONFIG_TARGET_SUPPORT_CPP11)
add_subdirectory(cpp11)
endif (NOT CONFIG_TARGET_SUPPORT_CPP11)


# Add "barbeque" target binary
set (BARBEQUE_SRC barbeque version object configuration_manager console_logger)
set (BARBEQUE_SRC platform_services dynamic_library ${BARBEQUE_SRC})
set (BARBEQUE_SRC plugin_manager modules_factory ${BARBEQUE_SRC})
set (BARBEQUE_SRC resource_manager application_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC resource_accounter ${BARBEQUE_SRC})
set (BARBEQUE_SRC application_proxy rpc_proxy rpc_messages ${BARBEQUE_SRC})
set (BARBEQUE_SRC platform_proxy ${BARBEQUE_SRC})
set (BARBEQUE_SRC signals_manager scheduler_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC synchronization_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC profile_manager ${BARBEQUE_SRC})
set (BARBEQUE_SRC daemonize ${BARBEQUE_SRC})
if (CONFIG_BBQUE_TEST_PLATFORM_DATA)
	set (BARBEQUE_SRC test_platform_data ${BARBEQUE_SRC})
else (CONFIG_BBQUE_TEST_PLATFORM_DATA)
 if (CONFIG_TARGET_LINUX)
	set (BARBEQUE_SRC pp/linux ${BARBEQUE_SRC})
 endif (CONFIG_TARGET_LINUX)
endif (CONFIG_BBQUE_TEST_PLATFORM_DATA)


add_executable (barbeque ${BARBEQUE_SRC})

# Linking dependencies
target_link_libraries(
	barbeque
	bbque_utils
	bbque_resources
	bbque_apps
	${Boost_LIBRARIES}
)

# Linking static plugins
target_link_libraries(
	barbeque
	-Wl,-whole-archive bbque_recipe_loader_xml -Wl,-no-whole-archive
)


# Linking dependencies for Generic-Linux targets
if (CONFIG_TARGET_LINUX)
target_link_libraries(
	barbeque
	${CGroup_LIBRARIES}
	-Wl,-whole-archive bbque_logger_log4cpp -Wl,-no-whole-archive
	-ldl -lrt
)
endif (CONFIG_TARGET_LINUX)

# Linking dependencies for Android-Linux targets
if (CONFIG_TARGET_ANDROID)
target_link_libraries(
	barbeque
	-Wl,-whole-archive bbque_cpp11 -Wl,-no-whole-archive
	-Wl,-whole-archive bbque_logger_android -Wl,-no-whole-archive
)
endif (CONFIG_TARGET_ANDROID)


# Use link path ad RPATH
set_property(TARGET barbeque PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

# Install the configuration file
install(FILES "${PROJECT_BINARY_DIR}/include/bbque/config.h"
		DESTINATION "${BBQUE_PATH_HEADERS}")

# Install the Barbeque target
install (TARGETS barbeque RUNTIME
		DESTINATION ${BBQUE_PATH_BBQ}
		COMPONENT BarbequeRTRM)
# Install the Barbeque configuration files
install (FILES "${PROJECT_BINARY_DIR}/config/bbque.conf"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeRTRM)
install (FILES "${PROJECT_BINARY_DIR}/config/bbque.conf_dbg"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeRTRM)
install (FILES "${PROJECT_BINARY_DIR}/config/bbque_2parma_demo2.conf"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeDEMOS)
# Install the daemon init script and its configuration file
install (PROGRAMS "${PROJECT_BINARY_DIR}/config/bbqued"
	DESTINATION ${BBQUE_PATH_INIT}
		COMPONENT BarbequeRTRM)
install (FILES "${PROJECT_BINARY_DIR}/config/default/bbqued"
	DESTINATION ${BBQUE_PATH_DEFAULTS}
		COMPONENT BarbequeRTRM)
# Install the BOSP Shell environment setup
install (FILES "${PROJECT_BINARY_DIR}/config/bosp_init.env"
		DESTINATION ${BBQUE_PATH_CONF}
		COMPONENT BarbequeRTRM)
# Install the PIL configurations
install (DIRECTORY "${PROJECT_SOURCE_DIR}/config/pil/"
		DESTINATION ${BBQUE_PATH_PILS}
		COMPONENT BarbequeRTRM
		FILES_MATCHING PATTERN "*.bpl")

