cmake_minimum_required(VERSION 3.14)

project(tsp)
set (CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(lib_sub)


file(GLOB tsp_SRC CONFIGURE_DEPENDS "src/*.c" )

set(time_SUB "lib_sub/time_sub.c")
set_source_files_properties(${tsp_SRC} PROPERTIES LANGUAGE C)
set_source_files_properties(${time_SUB} PROPERTIES LANGUAGE C)


add_executable(${PROJECT_NAME} ${tsp_SRC} ${time_SUB})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(/DDEBUG) #Whether debug version is needed
add_definitions(/Zi /MT /EHsc /Oy- /Ob0)

set(CONCORDE_LIB "lib_sub/")

set(CPLEX_PATH "C:/Program Files/IBM/ILOG/CPLEX_Studio2211/cplex")
set(CPLEX_LIB ${CPLEX_PATH}/lib/x64_windows_msvc14/stat_mda)
include_directories(${CPLEX_PATH}/include/ilcplex)
add_definitions(-D IL_STD)

target_link_directories(${PROJECT_NAME} PUBLIC ${CPLEX_LIB})
target_link_directories(${PROJECT_NAME} PUBLIC ${CONCORDE_LIB})
target_link_libraries(${PROJECT_NAME} cplex2211)
target_link_libraries(${PROJECT_NAME} ilocplex)
target_link_libraries(${PROJECT_NAME} mincut)
