cmake_minimum_required(VERSION 3.5)

project(benchmark CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
find_package(vsomeip3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS log system)

set(CMAKE_INSTALL_RPATH $ORIGIN)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(${VSOMEIP3_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})


add_subdirectory(socket)
add_subdirectory(someip)
