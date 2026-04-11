cmake_minimum_required(VERSION 4.1)
project(AirSpringSuspension)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 17)
add_executable(AirSpringSuspension main.cpp
        AirSpringThermodynamic.h
        AirSpringThermodynamic.cpp
)
