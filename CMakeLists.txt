cmake_minimum_required(VERSION 3.0)

# find_library(PTHREAD)
# set(CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
add_library(js_executor SHARED src/execClient.cpp src/executor.cpp)
target_include_directories(js_executor PUBLIC include)

add_executable(main src/main.cpp )
target_link_libraries(main js_executor pthread)
