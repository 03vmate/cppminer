cmake_minimum_required(VERSION 3.23)
project(mateminer)

set(CMAKE_CXX_STANDARD 23)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(mateminer main.cpp CurrentWorkUnit.cpp)

if(WIN32)
  target_link_libraries(mateminer wsock32 ws2_32)
endif()

target_link_libraries(mateminer ${Boost_LIBRARIES})