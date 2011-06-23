
#----- Add "sasb" target dynamic library
set(PLUGIN_SASB_SRC  sasb_syncpol sasb_plugin)
add_library(plugin_syncpol_sasb MODULE ${PLUGIN_SASB_SRC})

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

