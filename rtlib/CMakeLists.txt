
## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/bbque/version.cc.in"
	"${PROJECT_BINARY_DIR}/bbque/version.cc" @ONLY
)

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -ldl")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

set (RTLIB_SRC bbque_rtlib bbque_rpc ${PROJECT_BINARY_DIR}/bbque/version.cc)
set (RTLIB_SRC bbque_exc ${RTLIB_SRC})
if(BBQUE_RPC_FIFO)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBBQUE_RPC_FIFO")
	set (RTLIB_SRC rpc_fifo_client ${RTLIB_SRC})
endif(BBQUE_RPC_FIFO)

set (RTLIB_SRC ${RTLIB_SRC})
#----- Add "bbque_rtlib" target dinamic binary
add_library(bbque_rtlib SHARED ${RTLIB_SRC})

# Linking dependencies
target_link_libraries(
	bbque_rtlib
	bbque_utils
	${Boost_LIBRARIES}
)

# Install the RTLib
set_property (TARGET bbque_rtlib PROPERTY
		FRAMEWORK ON)
set_property (TARGET bbque_rtlib PROPERTY PUBLIC_HEADER
		${PROJECT_SOURCE_DIR}/include/bbque/rtlib.h
		${PROJECT_SOURCE_DIR}/include/bbque/rtlib/bbque_exc.h)
install (TARGETS bbque_rtlib LIBRARY
		DESTINATION ${BBQUE_PATH_RTLIB}
		PUBLIC_HEADER DESTINATION ${BBQUE_PATH_HEADERS}
		COMPONENT BarbequeRTLib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/bbque/utils"
		DESTINATION "${BBQUE_PATH_HEADERS}"
		FILES_MATCHING PATTERN "*.h")


