/*
Computer Graphics B2023
Roller Coaster Engine

Author: Alejandro Mujica
alejandro.j.mujic4@gmail.com

Author: Anthony Dugarte
toonny1998@gmail.com

Author: Kevin Márquez
marquezberriosk@gmail.com

Author: Lewis Ochoa
lewis8a@gmail.com

This file contains the game settings that include the association of the
inputs with an their ids, constants of values to set up the game, sounds,
textures, frames, and fonts.
*/

#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreTrays.h"
#include <OgreResourceGroupManager.h>
#include <OgreTextureManager.h>
#include <OgreImage.h>
#include <OgreDataStream.h>
#include <OgreConfigFile.h>
#include <fstream>
#include <iostream>

using namespace Ogre;
using namespace OgreBites;

std::string resourcesFile = "resources.cfg";