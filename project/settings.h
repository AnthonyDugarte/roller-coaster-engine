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

struct Settings
{
    static const fs::path MUSIC_PATH;
    static const fs::path FX_PATH;
    static sf::Music ambience,mainMenu;
    static std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    static std::unordered_map<std::string, sf::Sound> sounds;
    static void loadSounds();
    static void playMainMenuMusic();
    static void stopMainMenuMusic();
    static void playAmbienceMusic();
    static void stopAmbienceMusic();
    static void setMusicVolume(float);
    static void setFxVolume(float);
};

std::random_device rd;
std::mt19937 gen(rd());
std::unordered_map<std::string, sf::SoundBuffer> Settings::soundBuffers;
std::unordered_map<std::string, sf::Sound> Settings::sounds{};
const fs::path Settings::MUSIC_PATH{"assets/music/"};
const fs::path Settings::FX_PATH{"assets/fx/"};
sf::Music Settings::ambience{};
sf::Music Settings::mainMenu{};

void Settings::loadSounds()
{
    sf::SoundBuffer buffer;
    sf::Sound sound;
    
    if (!buffer.loadFromFile(Settings::FX_PATH / "ui/click.ogg"))
    {
        throw std::runtime_error{"Error loading sound ui/click.ogg"};
    }
    auto result = Settings::soundBuffers.emplace("click", buffer);
    sound.setBuffer(result.first->second);
    Settings::sounds["click"] = sound;

    if (!buffer.loadFromFile(Settings::FX_PATH / "ui/start.ogg"))
    {
        throw std::runtime_error{"Error loading sound ui/start.ogg"};
    }
    result = Settings::soundBuffers.emplace("start", buffer);
    sound.setBuffer(result.first->second);
    Settings::sounds["start"] = sound;

    if (!buffer.loadFromFile(Settings::FX_PATH / "ui/slider.ogg"))
    {
        throw std::runtime_error{"Error loading sound ui/slider.ogg"};
    }
    result = Settings::soundBuffers.emplace("slider", buffer);
    sound.setBuffer(result.first->second);
    Settings::sounds["slider"] = sound;

    if (!Settings::ambience.openFromFile(Settings::MUSIC_PATH / "ambience.ogg"))
    {
        throw std::runtime_error{"Error loading music assets/music/ambience.ogg"};
    }
    if (!Settings::mainMenu.openFromFile(Settings::MUSIC_PATH / "mainMenu.ogg"))
    {
        throw std::runtime_error{"Error loading music assets/music/mainMenu.ogg"};
    }
    std::cout<<"Holaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
}

void Settings::playMainMenuMusic()
{
    Settings::mainMenu.setLoop(true);
    Settings::mainMenu.play();
}

void Settings::stopMainMenuMusic()
{
    Settings::mainMenu.stop();
}

void Settings::playAmbienceMusic()
{
    Settings::ambience.setLoop(true);
    Settings::ambience.play();
}

void Settings::stopAmbienceMusic()
{
    Settings::mainMenu.stop();
}

void Settings::setMusicVolume(float volume)
{
    Settings::mainMenu.setVolume(volume);
    Settings::ambience.setVolume(volume);
}

void Settings::setFxVolume(float volume)
{
    Settings::sounds["click"].setVolume(volume);
    Settings::sounds["start"].setVolume(volume);
    Settings::sounds["slider"].setVolume(volume);
}