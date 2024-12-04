cmake_minimum_required(VERSION 3.10)
project(tsp_solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CPLEX
set(CPLEX_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio2211" CACHE PATH "CPLEX root directory")
find_path(CPLEX_INCLUDE_DIR
    NAMES ilcplex/cplex.h
    HINTS "${CPLEX_ROOT_DIR}/cplex/include"
)

find_library(CPLEX_LIBRARY
    NAMES cplex
    HINTS "${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic"
)

find_library(ILOCPLEX_LIBRARY
    NAMES ilocplex
    HINTS "${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic"
)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${CPLEX_INCLUDE_DIR}
)

# Add source files
set(SOURCES
    src/main.cpp
    src/model.cpp
)

# Create executable
add_executable(tsp_solver ${SOURCES})

# Link libraries
target_link_libraries(tsp_solver
    ${CPLEX_LIBRARY}
    ${ILOCPLEX_LIBRARY}
    pthread
    dl
)
