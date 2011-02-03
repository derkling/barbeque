

#----- Add "testing" target static library
set(PLUGIN_DUMMY_SRC  dummy_test dummy_plugin)
add_library(plugin_test_dummy STATIC ${PLUGIN_DUMMY_SRC})


#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

