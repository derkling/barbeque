

#----- Add "testing" target static library
set(PLUGIN_TESTING_SRC  static_test_object static_test_plugin)
add_library(plugin_testingobject STATIC ${PLUGIN_TESTING_SRC})


#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

