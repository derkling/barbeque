# Sets the include path for libxml
set (LIBXML2_INCLUDE_PATH "/usr/include/libxml2")

# Include libxml headers
include_directories (
	${LIBXML2_INCLUDE_PATH}
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set (MONITORS_SRC time_monitor ${PROJECT_BINARY_DIR}/bbque/version.cc)
set (MONITORS_SRC throughput_monitor memory_monitor ${MONITORS_SRC})
set (MONITORS_SRC op_manager ${MONITORS_SRC})

#----- Add "bbque_monitors" target dinamic binary
add_library(bbque_monitors SHARED ${MONITORS_SRC})

# Linking dependencies
target_link_libraries(
	bbque_monitors
	${Boost_LIBRARIES}
	-ldl -lrt -lxml2
)

# Use link path ad RPATH
set_property (TARGET bbque_monitors PROPERTY
		INSTALL_RPATH_USE_LINK_PATH TRUE)

set_property (TARGET bbque_monitors PROPERTY
		FRAMEWORK ON)

set_property (TARGET bbque_monitors PROPERTY PUBLIC_HEADER
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/generic_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/time_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/time_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/throughput_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/throughput_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/memory_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/temperature_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/metric_priority.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/operating_point.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/op_filter.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/op_manager.h)


# Install Monitors
install (TARGETS bbque_monitors LIBRARY
		DESTINATION ${BBQUE_PATH_RTLIB}
		PUBLIC_HEADER DESTINATION "${BBQUE_PATH_HEADERS}/monitors"
		COMPONENT BarbequeRTLib)
