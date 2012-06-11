
#----- Add "RPC FIFO" target dynamic library
set(PLUGIN_RPC_FIFO_SRC  fifo_rpc fifo_plugin)
add_library(bbque_rpc_fifo MODULE ${PLUGIN_RPC_FIFO_SRC})
install(TARGETS bbque_rpc_fifo LIBRARY
		DESTINATION ${BBQUE_PATH_PLUGINS}
		COMPONENT BarbequeRTRM)

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

