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
file(COPY zip DESTINATION .)
file(COPY model DESTINATION .)
file(COPY img DESTINATION .)
file(COPY texture DESTINATION .)


# Executable
add_executable(RollerCoasterEngine RollerCoasterEngine.cpp )
target_link_libraries(
RollerCoasterEngine 
OgreBites
${OIS_LIBRARIES}
${OGRE_Overlay_LIBRARIES}
${OGRE_Terrain_LIBRARIES})