
#----- Add "random" target static library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PLUGIN_RANDOM_SRC  random_schedpol random_plugin)
	add_library(plugin_schedpol_random MODULE ${PLUGIN_RANDOM_SRC})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

#----- Add "yamca" target static library
set(PLUGIN_YAMCA_SRC  yamca_schedpol yamca_plugin)
add_library(plugin_schedpol_yamca MODULE ${PLUGIN_YAMCA_SRC})

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

