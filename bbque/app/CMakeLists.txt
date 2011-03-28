# Add compilation dependencies
include_directories(
	${PROJECT_BINARY_DIR}/include/app
	${PROJECT_SOURCE_DIR}/include/app
)

# Configure the building options for each release
# Common
#set (CMAKE_CXX_FLAGS "-Wall")
# DEBUGGING
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wextra -pedantic")
set (BBQUE_DEBUG ON)
# DESKTOP RELEASE
#set (CMAKE_CXX_FLAGS_RELEASE "-O3")
# EMBEDDED RELEASE
#set (CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

# Add sources in the current directory to the target binary
set (APPLICATION_SRC application)
set (APPLICATION_SRC working_mode ${APPLICATION_SRC})
set (APPLICATION_SRC recipe ${APPLICATION_SRC})
set (APPLICATION_SRC plugin_data ${APPLICATION_SRC})

#Add as library
add_library(application_objs STATIC ${APPLICATION_SRC})

