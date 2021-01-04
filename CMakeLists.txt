cmake_minimum_required(VERSION 3.7)
project(POS_semestral_project C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -pthread")
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(POS_semestral_project cloveceLogic.c server.c)
target_link_libraries(POS_semestral_project ${CURSES_LIBRARIES})