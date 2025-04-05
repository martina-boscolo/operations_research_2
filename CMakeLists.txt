cmake_minimum_required(VERSION 3.10)
project(TSP_SOLVER C)

# Set C standard
set(CMAKE_C_STANDARD 11)

# Set directories
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Set CPLEX directories
set(CPLEX_DIR "C:/Program Files/IBM/ILOG/CPLEX_Studio2212")
set(CPLEX_INCLUDE_DIR "${CPLEX_DIR}/cplex/include")
set(CPLEX_LIB_DIR "${CPLEX_DIR}/cplex/lib/x64_windows_msvc14")

# Set specific CPLEX library path
set(CPLEX_LIB_FILE "${CPLEX_LIB_DIR}/stat_mda/cplex2212.lib")

# Find source files
file(GLOB SRC_FILES "${SRC_DIR}/*.c")

# Create executable
add_executable(tsp ${SRC_FILES})

# Add include directories
target_include_directories(tsp PRIVATE ${INCLUDE_DIR} ${CPLEX_INCLUDE_DIR})

# Link libraries - Use the full library path
target_link_libraries(tsp "${CPLEX_LIB_FILE}")

# Add compiler flags (adjusted for MSVC)
if(MSVC)
  # Disable runtime checks when optimization is enabled
  string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  
  # Set warning level
  target_compile_options(tsp PRIVATE /W4)
  

  # Set console subsystem
  set_target_properties(tsp PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
else()
  target_compile_options(tsp PRIVATE -Wall -Wextra -O3)
endif()

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()