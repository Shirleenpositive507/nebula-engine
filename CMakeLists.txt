cmake_minimum_required(VERSION 3.16)
project(NebulaEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

find_package(SFML 2.5 REQUIRED COMPONENTS system window graphics audio network)

add_subdirectory(engine)
add_subdirectory(tools)

add_executable(nebula main.cpp)
target_link_libraries(nebula PRIVATE nebula_core ${SFML_LIBRARIES})
