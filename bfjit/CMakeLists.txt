# CMakeList.txt : CMake project for bfjit, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (bfjit "Main.c"  "Compile.h" "Compile.c" "Assemble.c" "Assemble.h" "extern_data.h" "InstructionSet.h" "list.c")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND CMAKE_BUILD_TYPE MATCHES "Release")

target_compile_options(bfjit PRIVATE /Zi)

set_target_properties(bfjit PROPERTIES LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")

set_target_properties(bfjit PROPERTIES COMPILE_PDB_NAME bfjit COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()
# TODO: Add tests and install targets if needed.
