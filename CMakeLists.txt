cmake_minimum_required(VERSION 4.0)

project(
  tapesort
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD             23)
set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

find_package(Lyra REQUIRED)
find_package(tomlplusplus REQUIRED)

add_library(tape STATIC
  src/tape/ITape.cpp
  src/tape/FileTape.cpp
)
target_include_directories(tape
  PUBLIC ${PROJECT_SOURCE_DIR}/include
)

add_library(sort STATIC
  src/sort/ExternalSort.cpp
)
target_include_directories(sort
  PUBLIC ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(sort PRIVATE tape)

add_executable(tapesort src/main.cpp)
target_link_libraries(tapesort PRIVATE tape)
target_link_libraries(tapesort PRIVATE sort)
target_link_libraries(tapesort PRIVATE bfg::lyra)
target_link_libraries(tapesort PRIVATE tomlplusplus::tomlplusplus)

add_executable(filetape src/filetape.cpp)
target_link_libraries(filetape PRIVATE tape)
target_link_libraries(filetape PRIVATE bfg::lyra)
