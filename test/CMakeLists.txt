
# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")
if(BBQUE_DEBUG)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_DEBUG")
	set(BBQUE_TESTS_SRC test_test)
endif(BBQUE_DEBUG)

#----- Add thereafter all the regression tests we want to run
set(BBQUE_TESTS_SRC test_all test_constraints ${BBQUE_TESTS_SRC})


#----- Add "bbque_tests" target application
set(BBQUE_TESTS_SRC ${BBQUE_TESTS_SRC})
#add_executable(bbque_tests ${BBQUE_TESTS_SRC})

create_test_sourcelist(BBQUE_TESTS_LIST bbque_test.cc ${BBQUE_TESTS_SRC})

# Add executable test driver
add_executable(bbque_tests ${BBQUE_TESTS_LIST})

# Linking dependencies
target_link_libraries(
	bbque_tests
	bbque_rtlib
)

set (BBQUE_TESTS_TO_RUN ${BBQUE_TESTS_SRC})
remove (BBQUE_TESTS_TO_RUN ${bbque_tests})

foreach(TEST ${BBQUE_TESTS_TO_RUN})
	get_filename_component(TEST_NAME ${TEST} NAME_WE)
	add_test(${TEST_NAME} ${CXX_TEST_PATH}/bbque_test ${TEST_NAME})
endforeach(TEST)


