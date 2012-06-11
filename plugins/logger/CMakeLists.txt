

if (CONFIG_EXTERNAL_LOG4CPP)

#----- Check for required LOG4CPP library
find_package(Log4CPP REQUIRED)

# Add compilation dependencies
include_directories(${log4cpp_INCLUDE_DIRS})

# Add linking dependencies
link_directories(${log4cpp_LIBRARY_DIRS})

#----- Add target static library
set(PLUGIN_LOG4CPP_SRC  log4cpp_logger log4cpp_plugin)
add_library(bbque_logger_log4cpp STATIC ${PLUGIN_LOG4CPP_SRC})

#----- Add library specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

#----- Linking dependencies
target_link_libraries(bbque_logger_log4cpp ${LOG4CPP_LIBRARIES})

endif (CONFIG_EXTERNAL_LOG4CPP)

