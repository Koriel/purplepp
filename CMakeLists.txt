cmake_minimum_required(VERSION 3.8)
project(purplepp)

set(CMAKE_CXX_STANDARD 11)

set(PURPLEPP_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(PURPLEPP_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(PURPLEPP_LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(PURPLEPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
#set(FMT_INCLUDE_DIRS ${SKYPEPP_EXTERNAL_DIR}/fmt)
#set(JSON_INCLUDE_DIRS ${SKYPEPP_EXTERNAL_DIR}/json)
#set(BUILD_CPR_TESTS "OFF")

#add_subdirectory(external/cpr)
#add_subdirectory(external/fmt)
add_subdirectory(src)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(purplepp_dev_test main.cpp)
    target_link_libraries(purplepp_dev_test purplepp)
    target_include_directories(purplepp_dev_test PRIVATE ${PURPLEPP_INCLUDE_DIRS})
endif()
