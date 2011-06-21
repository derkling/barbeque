
#----- Add "RPC FIFO" target dynamic library
set(PLUGIN_RPC_FIFO_SRC  fifo_rpc fifo_plugin)
add_library(plugin_rpc_fifo MODULE ${PLUGIN_RPC_FIFO_SRC})

#----- Add "testing" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

