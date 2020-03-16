cmake_minimum_required(VERSION 3.0)

# find_library(PTHREAD)
# set(CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(LINK_DIR lib)
link_directories((${LINK_DIR}))

add_library(chakra_exec SHARED src/execClient.cpp src/executor.cpp src/c_api.cpp)
target_include_directories(chakra_exec PUBLIC include include/chakra)
target_link_libraries(chakra_exec ChakraCore)

add_executable(main src/main.cpp)
target_link_libraries(main chakra_exec pthread)
