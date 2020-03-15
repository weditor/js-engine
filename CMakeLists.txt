cmake_minimum_required(3)

add_library(js_executor src/execClient.cpp src/executor.cpp)
target_include_directories(js_executor include)
