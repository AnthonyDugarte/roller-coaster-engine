#########################################################
#Roller Coaster Engine
#########################################################

# Setup
cmake_minimum_required (VERSION 2.8)
project(RollerCoasterEngine)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF) 
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# SFML
find_package(SFML 2.5 COMPONENTS system audio REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

# Resources
find_package(OGRE REQUIRED COMPONENTS Bites CONFIG)
configure_file(resources.cfg resources.cfg COPYONLY)
file(COPY assets/material DESTINATION ./assets)
file(COPY assets/img/backgroundMain DESTINATION ./assets/img)
file(COPY assets/img/gui DESTINATION ./assets/img)
file(COPY assets/img/logo DESTINATION ./assets/img)
file(COPY assets/img/sky DESTINATION ./assets/img)
file(COPY assets/texture DESTINATION ./assets)
file(COPY assets/model DESTINATION ./assets)
file(COPY assets/fonts DESTINATION ./assets)
file(COPY assets/music DESTINATION ./assets)


# Executable
add_executable(${PROJECT_NAME} RollerCoasterEngine.cpp)
target_link_libraries(
${PROJECT_NAME}
OgreBites
${OIS_LIBRARIES}
${OGRE_Overlay_LIBRARIES}
${OGRE_Terrain_LIBRARIES})
target_link_libraries(${PROJECT_NAME} sfml-audio)
