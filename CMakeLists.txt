cmake_minimum_required(VERSION 3.7)
project(POS_semestral_project C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_library(sharedLib SHARED shared.c shared.h)
add_executable(runServer server.c)
add_executable(runClient client.c)

target_link_libraries(runServer sharedLib)
target_link_libraries(runClient sharedLib)
target_link_libraries(runClient ${CURSES_LIBRARIES})