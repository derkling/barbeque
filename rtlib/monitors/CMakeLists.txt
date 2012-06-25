

# FIXME: avoid definition of "NDEBUG" when  compiling on release mode, which
# produces a segfault on shared_pts and functors usages. To be verified.
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

set (MONITORS_SRC time_monitor ${PROJECT_BINARY_DIR}/bbque/version.cc)
set (MONITORS_SRC throughput_monitor memory_monitor ${MONITORS_SRC})
set (MONITORS_SRC run_time_manager ${MONITORS_SRC})
set (MONITORS_SRC op_manager ${MONITORS_SRC})

# Copy headers where they are expected by applications
file (COPY "${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors"
	DESTINATION "${PROJECT_BINARY_DIR}/include/bbque"
	FILES_MATCHING PATTERN "*.h")

#----- Add "bbque_monitors" target dinamic binary
add_library(bbque_monitors SHARED ${MONITORS_SRC})

# Linking dependencies
target_link_libraries(
	bbque_monitors
	bbque_rtlib
	${Boost_LIBRARIES}
)

# Add "Generic-Linux" specific flags
if (CONFIG_TARGET_LINUX)
target_link_libraries(
	bbque_monitors
	-ldl -lrt
)
endif (CONFIG_TARGET_LINUX)

# Use link path ad RPATH
set_property (TARGET bbque_monitors PROPERTY
		INSTALL_RPATH_USE_LINK_PATH TRUE)

set_property (TARGET bbque_monitors PROPERTY
		FRAMEWORK ON)

set_property (TARGET bbque_monitors PROPERTY PUBLIC_HEADER
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/goal_info.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/generic_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/time_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/time_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/throughput_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/throughput_window.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/memory_monitor.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/metric_priority.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/operating_point.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/op_filter.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/op_manager.h
	${PROJECT_SOURCE_DIR}/include/bbque/rtlib/monitors/run_time_manager.h)


# Install Monitors
install (TARGETS bbque_monitors LIBRARY
		DESTINATION ${BBQUE_PATH_RTLIB}
		PUBLIC_HEADER DESTINATION "${BBQUE_PATH_HEADERS}/monitors"
		COMPONENT BarbequeRTLib)

# Force RTLib being build before these modules
add_dependencies(bbque_monitors bbque_rtlib)
