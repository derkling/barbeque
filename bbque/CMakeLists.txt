
## Add package version definition
configure_file ("${PROJECT_SOURCE_DIR}/bbque/version.cpp.in" "${PROJECT_BINARY_DIR}/bbque/version.cpp" @ONLY)

# Add binary "barbeque".
set (BARBEQUE_SRC main version)
add_executable (barbeque ${BARBEQUE_SRC})

