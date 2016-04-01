cmake_minimum_required(VERSION 3.2)

project(ibotpp)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Threads REQUIRED)
find_package(Boost REQUIRED system thread)
find_package(LLVM REQUIRED
	core analysis support
)
find_package(Clang REQUIRED
	clangAnalysis clangAST clangCodeGen clangDriver clangFrontend clangParse
	clangRewrite clangSema clangSerialization clangTooling clangEdit clangLex
	clangBasic
)

include_directories(include ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS)

add_executable(ibotpp src/main.cpp)
set_property(TARGET ibotpp PROPERTY CXX_STANDARD 11)
set_property(TARGET ibotpp PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(ibotpp ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${CLANG_LIBS} ${LLVM_LIBS} ncurses dl)
