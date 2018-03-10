cmake_minimum_required(VERSION 3.9)
project(BitmapLogicSimulatorClone)

set(CMAKE_CXX_STANDARD 17)

add_executable(BitmapLogicSimulatorClone BMPC.cpp)

find_package(SFML REQUIRED system window graphics)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(BitmapLogicSimulatorClone ${SFML_LIBRARIES})
endif()
