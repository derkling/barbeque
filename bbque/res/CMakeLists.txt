# Add compilation dependencies
include_directories(
	${PROJECT_BINARY_DIR}/include/res
	${PROJECT_SOURCE_DIR}/include/res
)

# Add sources in the current directory to the target binary
set (RESOURCES_SRC resources)
set (RESOURCES_SRC resource_tree ${RESOURCES_SRC})
set (RESOURCES_SRC usage ${RESOURCES_SRC})

#Add as library
add_library(bbque_resources STATIC ${RESOURCES_SRC})
