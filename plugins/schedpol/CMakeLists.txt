
#----- Add "yamca" target static library
set(PLUGIN_YAMCA_SRC  yamca_schedpol yamca_plugin)
add_library(plugin_schedpol_yamca MODULE ${PLUGIN_YAMCA_SRC})

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

