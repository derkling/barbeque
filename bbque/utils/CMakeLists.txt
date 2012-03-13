
# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -fPIC")

# Add sources in the current directory to the target binary
set (BBQUE_UTILS_SRC timer deferrable)
set (BBQUE_UTILS_SRC ${BBQUE_UTILS_SRC} perf metrics_collector)
set (BBQUE_UTILS_SRC ${BBQUE_UTILS_SRC} attributes_container)

#Add as library
add_library(bbque_utils STATIC ${BBQUE_UTILS_SRC})

