##############################################################################
#
# Simple CMakeLists.txt example for cmake. 
#
# This file will initiate compilation of main.cpp into main executable.
#
# Change project name and list of SOURCES for your application.
#
##############################################################################

cmake_minimum_required (VERSION 3.5)

## project name
project( main ) # CXX or C )

## generate compile_commands.json, must be after project
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( SOURCES ${CMAKE_PROJECT_NAME} main.cpp )

## executable in source directory
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )

## no gnu++..
set(CMAKE_CXX_EXTENSIONS OFF)
## c++11
set( CMAKE_CXX_STANDARD 11 )

## compiler options
add_compile_options( -Wall -g -pthread )

## linker option -l, since cmake #3.13 :-(
#add_link_options( -lm )

#add_definitions( -DMACRO=1 )

## linker option -L
#link_directories( /tmp )

## cc option -I
#include_directories( /tmp )

link_libraries( m rt pthread )

## add package 
#find_package( OpenCV )
#include_directories( ${OpenCV_INCLUDE_DIR} )
#link_libraries( ${OpenCV_LIBS} )

add_executable( ${CMAKE_PROJECT_NAME} ${SOURCES} )

#target_include_directories( ${CMAKE_PROJECT_NAME} PUBLIC /usr/include )
#target_link_libraries( ${CMAKE_PROJECT_NAME} z )

#target_compile_options( ${CMAKE_PROJECT_NAME} PUBLIC -O0 )


