cmake_minimum_required(VERSION 3.9)
project(BitmapLogicSimulatorClone)

set(CMAKE_CXX_STANDARD 17)

add_executable(BitmapLogicSimulatorClone BMPC.cpp)

set(CMAKE_CXX_FLAGS "-O3")

target_link_libraries(Junk sfml-graphics sfml-window sfml-system)
