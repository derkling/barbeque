
# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")
if(BBQUE_DEBUG)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_DEBUG")
endif(BBQUE_DEBUG)


#----- Add "dummy_app" target application
set(RTLIB_TUTORIAL_SRC main app)
add_executable(tutorial ${RTLIB_TUTORIAL_SRC})

# Linking dependencies

target_link_libraries(
	tutorial
	bbque_rtlib
)
