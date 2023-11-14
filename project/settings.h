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
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <OgreTimer.h>

#include <SFML/Audio.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>

namespace fs = std::filesystem;
using namespace Ogre;
using namespace OgreBites;

std::random_device rd;
std::mt19937 gen(rd());

struct Settings
{
    static const fs::path SOUNDS_PATH;
    static sf::Music music;
    
    static void init();
    static void load_sounds();
    static void play_sound();
};

const fs::path Settings::SOUNDS_PATH{"assets/music/"};
sf::Music Settings::music{};

void Settings::init()
{
    Settings::load_sounds();
}

void Settings::load_sounds()
{
    if (!Settings::music.openFromFile(Settings::SOUNDS_PATH / "marios_way.ogg"))
    {
        throw std::runtime_error{"Error loading music assets/music/marios_way.ogg"};
    }
}

void Settings::play_sound()
{
    Settings::music.setLoop(true);
    Settings::music.play();
}
