cmake_minimum_required(VERSION 3.7)
project(purplepp)

set(CMAKE_CXX_STANDARD 14)

set(PURPLEPP_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(PURPLEPP_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(PURPLEPP_LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(PURPLEPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")
set(FMT_INCLUDE_DIRS ${PURPLEPP_EXTERNAL_DIR}/fmt)

add_subdirectory(external/fmt)
add_subdirectory(src)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(purplepp_dev_test main.cpp)
    target_link_libraries(purplepp_dev_test purplepp)
    target_include_directories(purplepp_dev_test PRIVATE ${PURPLEPP_INCLUDE_DIR})
endif()
