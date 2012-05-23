

# Add sources in the current directory to the target binary
set (BBQUE_CPP11_SRC gnu_chrono)
set (BBQUE_CPP11_SRC ${BBQUE_CPP11_SRC} gnu_mutex gnu_condition_variable)
set (BBQUE_CPP11_SRC ${BBQUE_CPP11_SRC} gnu_thread gnu_future)

# Add as library
add_library(bbque_cpp11 STATIC ${BBQUE_CPP11_SRC})

