
# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -ldl")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")
if(BBQUE_DEBUG)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_DEBUG")
endif(BBQUE_DEBUG)



#----- Add "rtlib_example01" target application
set(RTLIB_EXAMPLE01_SRC bbque_exc timer example01)
add_executable(rtlib_example01 ${RTLIB_EXAMPLE01_SRC})

# Linking dependencies
target_link_libraries(
	rtlib_example01
	bbque_rtlib
)

#----- Add "rtlib_testapp" target application
set(RTLIB_TESTAPP_SRC bbque_exc timer main)
add_executable(rtlib_testapp ${RTLIB_TESTAPP_SRC})

# Linking dependencies
target_link_libraries(
	rtlib_testapp
	bbque_rtlib
)

