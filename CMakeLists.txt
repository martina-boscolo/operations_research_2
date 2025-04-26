cmake_minimum_required(VERSION 3.10)
project(TSP_SOLVER C)

# Set C standard
set(CMAKE_C_STANDARD 11)

# Set directories
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Concorde directory (all files flat inside)
set(CONCORDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/concorde")

# Set CPLEX directories
set(CPLEX_DIR "C:/Program Files/IBM/ILOG/CPLEX_Studio2212")
set(CPLEX_INCLUDE_DIR "${CPLEX_DIR}/cplex/include")
set(CPLEX_LIB_DIR "${CPLEX_DIR}/cplex/lib/x64_windows_msvc14")

# Set specific CPLEX library path
set(CPLEX_LIB_FILE "${CPLEX_LIB_DIR}/stat_mda/cplex2212.lib")

# Find your project's source files
file(GLOB SRC_FILES "${SRC_DIR}/*.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

# Manually add needed Concorde source files
set(CONCORDE_FILES
    "${CONCORDE_DIR}/mincut.cpp"
    # Add more .c files if needed from concorde/
)

# Create executable
add_executable(tsp ${SRC_FILES} ${CONCORDE_FILES})

# Add include directories (your includes + concorde's headers if any)
target_include_directories(tsp PRIVATE ${INCLUDE_DIR} ${CONCORDE_DIR} ${CPLEX_INCLUDE_DIR})

# Link directories for concorde (to find mincut.lib)
target_link_directories(tsp PRIVATE ${CONCORDE_DIR})

# Link directories for CPLEX
target_link_directories(tsp PRIVATE "${CPLEX_LIB_DIR}/stat_mda")

# Link libraries
target_link_libraries(tsp "${CPLEX_LIB_FILE}")
target_link_libraries(tsp ilocplex mincut)

# Add compiler definitions
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D IL_STD)
add_definitions(-DDEBUG)

# Compiler flags
if(MSVC)
  string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  target_compile_options(tsp PRIVATE /W4 /Zi /MT /EHsc /Oy- /Ob0)
  set_target_properties(tsp PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
else()
  target_compile_options(tsp PRIVATE -Wall -Wextra -O3)
endif()

# Default to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
