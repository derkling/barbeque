##
# This builds the class implementing the computation of the
# scheduling metrics by componentes
##

# Add sources in the current directory to the target binary
set (SCHED_CONTRIB_SRC sched_contrib_manager)
set (SCHED_CONTRIB_SRC sched_contrib ${SCHED_CONTRIB_SRC})
set (SCHED_CONTRIB_SRC sc_value ${SCHED_CONTRIB_SRC})
set (SCHED_CONTRIB_SRC sc_reconfig ${SCHED_CONTRIB_SRC})
set (SCHED_CONTRIB_SRC sc_congestion ${SCHED_CONTRIB_SRC})
set (SCHED_CONTRIB_SRC sc_fairness ${SCHED_CONTRIB_SRC})

# Add as library
add_library(bbque_sched_contribs STATIC ${SCHED_CONTRIB_SRC})
set_target_properties(bbque_sched_contribs PROPERTIES LINKER_LANGUAGE CXX)

