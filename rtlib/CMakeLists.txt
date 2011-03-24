
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)

# Check for required libs and packages (headers + lib)
find_package(Boost 1.40.0 REQUIRED system program_options filesystem)

# Add compilation dependencies
include_directories(
	${Boost_INCLUDE_DIRS}
)

# Add linking dependencies
link_directories(
	${Boost_LIBRARY_DIRS}
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -ldl")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

set (RTLIB_SRC bbque_rtlib ${PROJECT_BINARY_DIR}/bbque/version.cc)
set (RTLIB_SRC ${RTLIB_SRC})
#----- Add "bbque_rtlib" target dinamic binary
add_library(bbque_rtlib SHARED ${RTLIB_SRC})

# Linking dependencies
target_link_libraries(
	bbque_rtlib
	${Boost_LIBRARIES}
)

