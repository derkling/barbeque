##
# This builds the class implementing the computation of the
# scheduling metrics by componentes
##

# Add "barbeque" specific flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -lrt -fPIC")

# Add sources in the current directory to the target binary
set (METRICS_CONTRIB_SRC metrics_contribute)
set (METRICS_CONTRIB_SRC mct_value ${METRICS_CONTRIB_SRC})
set (METRICS_CONTRIB_SRC mct_reconfig ${METRICS_CONTRIB_SRC})
set (METRICS_CONTRIB_SRC mct_congestion ${METRICS_CONTRIB_SRC})

# Add as library
add_library(bbque_sched_metrics STATIC ${METRICS_CONTRIB_SRC})
set_target_properties(bbque_sched_metrics PROPERTIES LINKER_LANGUAGE CXX)

