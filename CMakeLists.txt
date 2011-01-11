
cmake_minimum_required (VERSION 2.6)

# The name of this project is "BARBEQUE".
project (BARBEQUE)

# Load additional CMake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake/")
include(GetGitRevisionDescription)
git_describe(GIT_SHA1)

# Make sure the compiler can find include files
include_directories (${PROJECT_SOURCE_DIR}/include)

# Recurse into these subfolders
add_subdirectory (bbque)
#add_subdirectory (plugins)

