#########################################################
#Roller Coaster Engine
#########################################################

# Setup
cmake_minimum_required (VERSION 2.8)
project(RollerCoasterEngine)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 11)

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


# Executable
add_executable(${PROJECT_NAME} RollerCoasterEngine.cpp)
target_link_libraries(
${PROJECT_NAME}
OgreBites
${OIS_LIBRARIES}
${OGRE_Overlay_LIBRARIES}
${OGRE_Terrain_LIBRARIES})