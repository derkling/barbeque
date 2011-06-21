
#----- Add "testing" target dynamic library
set(PLUGIN_TEST_COREINT_SRC  coreint_test coreint_plugin)
add_library(plugin_test_coreint MODULE ${PLUGIN_TEST_COREINT_SRC})

#----- Add "testing" target static library
set(PLUGIN_DUMMY_SRC  dummy_test dummy_plugin)
#add_library(plugin_test_dummy STATIC ${PLUGIN_DUMMY_SRC})

#----- Add "testing" target dynamic library
add_library(plugin_test_dummy_so MODULE ${PLUGIN_DUMMY_SRC})
set_target_properties(plugin_test_dummy_so PROPERTIES COMPILE_FLAGS "-DBBQUE_DYNAMIC_PLUGIN")

#----- Add "testing" target dynamic library
set(PLUGIN_TEST_APROX_SRC  aprox_test aprox_plugin)
add_library(plugin_test_aprox MODULE ${PLUGIN_TEST_APROX_SRC})

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

