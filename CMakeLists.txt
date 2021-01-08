cmake_minimum_required(VERSION 3.7)
project(POS_semestral_project C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(runServer server.c server.h)
add_executable(runClient client.h client.c)
target_link_libraries(runClient ${CURSES_LIBRARIES})