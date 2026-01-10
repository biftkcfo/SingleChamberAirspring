cmake_minimum_required(VERSION 4.1)
project(SingleChamberAirSpring)

set(CMAKE_CXX_STANDARD 20)

add_executable(SingleChamberAirSpring main.cpp
        AirSpringThermodynamic.h
        AirSpringThermodynamic.cpp)
