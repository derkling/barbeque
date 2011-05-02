
# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -ldl")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

#----- Add "dummy_app" target application
set(RTLIB_TESTAPP_SRC app timer main)
add_executable(rtlib_testapp ${RTLIB_TESTAPP_SRC})

# Linking dependencies
target_link_libraries(
	rtlib_testapp
	bbque_rtlib
)

