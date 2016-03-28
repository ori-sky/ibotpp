cmake_minimum_required(VERSION 3.2)

project(ibotpp)
include_directories(include)
add_executable(ibotpp src/main.cpp)
set_property(TARGET ibotpp PROPERTY CXX_STANDARD 11)
set_property(TARGET ibotpp PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(ibotpp pthread ssl boost_system boost_thread)
