# Add compilation dependencies
include_directories(
	${PROJECT_BINARY_DIR}/include/app
	${PROJECT_SOURCE_DIR}/include/app
)

# Add sources in the current directory to the target binary
set (APPLICATION_SRC application working_mode recipe)

#Add as library
add_library(apps STATIC ${APPLICATION_SRC})

#Link static library
target_link_libraries(apps bbque_utils)
