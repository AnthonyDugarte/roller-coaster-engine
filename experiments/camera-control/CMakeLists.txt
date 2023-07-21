cmake_minimum_required (VERSION 2.8)
project(OgreTutorialsSample)

# required for Ogre 1.11+
set(CMAKE_CXX_STANDARD 11)

find_package(OGRE REQUIRED COMPONENTS Bites CONFIG)

add_executable(CC CameraControl.cpp)
target_link_libraries(CC OgreBites)
