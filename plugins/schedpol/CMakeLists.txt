
#----- Add "yamca" target static library
set(PLUGIN_YAMCA_SRC  yamca_schedpol yamca_plugin)
add_library(plugin_schedpol_yamca_so MODULE ${PLUGIN_YAMCA_SRC})
set_target_properties(plugin_schedpol_yamca_so PROPERTIES COMPILE_FLAGS "-DBBQUE_DYNAMIC_PLUGIN")

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

