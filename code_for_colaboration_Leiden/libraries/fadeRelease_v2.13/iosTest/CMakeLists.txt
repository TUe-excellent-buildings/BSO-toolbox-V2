cmake_minimum_required(VERSION 3.21)

# Name of the project
project(example_ios)
set(CMAKE_VERBOSE_MAKEFILE ON)

# GLOBBING
set (SD ${CMAKE_CURRENT_SOURCE_DIR}/../examples_2D/)
set (SA_SRC ${SD}main.cpp ${SD}ex0_hello_triangulation.cpp ${SD}ex1_benchmark.cpp ${SD}ex2_access_draw.cpp ${SD}ex3_constraint_edges.cpp ${SD}ex4_zones.cpp ${SD}ex5_boolean_ops.cpp ${SD}ex6_boolean_ops2.cpp ${SD}ex7_quality_meshing.cpp ${SD}ex8_triangulation_export.cpp ${SD}ex9_random_objects.cpp ${SD}ex10_progress_bar.cpp ${SD}ex11_save_and_load.cpp ${SD}ex12_voronoi.cpp )
set(CMAKE_MACOSX_RPATH 1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include_fade2d/)

MESSAGE("INCDIR=${CMAKE_CURRENT_SOURCE_DIR}/../include_fade2d/")
link_directories(../lib_ios/)
add_definitions(-std=c++11 -pthread -fstrict-aliasing -Wunused-local-typedef  -Wno-unused-local-typedefs -Wno-long-long -O3 -Wextra -Wunused -Wall -pedantic-errors  -funroll-loops)

# Add the dt2-executable
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
set(EXNAME fadeExample_ios_${PLATFORM})
add_executable(${EXNAME}  ${SA_SRC})
set_target_properties(${EXNAME} PROPERTIES
          BUNDLE True
          MACOSX_BUNDLE_GUI_IDENTIFIER Fade_Example
          MACOSX_BUNDLE_BUNDLE_NAME Fade_Example_Bundle
          MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
          )

TARGET_LINK_LIBRARIES(${EXNAME} ${CMAKE_CURRENT_SOURCE_DIR}/../lib_ios/libgmp ${CMAKE_CURRENT_SOURCE_DIR}/../lib_ios/libfade2d_arm_x64 pthread )

