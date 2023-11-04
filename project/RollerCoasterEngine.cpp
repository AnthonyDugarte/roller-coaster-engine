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

This file contains the design of GUI the game.
*/

#include "settings.h"

class RollerCoaster:
    public ApplicationContext,  // Base class responsible for setting up a common context for applications
    public InputListener,       // Needed to manage events mouse, keyboard, ...
    public TrayListener         // Needed to manage events in trays (buttons, layers, sliders, ...)
{
    public:
        //Basic
        RollerCoaster();
        virtual ~RollerCoaster() {}
        void setup();

        //GUI
        void menuGUI();
        void creditsGUI();

        //Interface
        void buttonHit(Button *	button);

        //Tool
        void loadResource();
        
    private:
        SceneManager* scnMgr;
        TrayManager* mTrayMgr;        
};

// START BASIC
RollerCoaster::RollerCoaster() :
    ApplicationContext("Roller Coaster Engine"),
    scnMgr{nullptr},
    mTrayMgr{nullptr}
{}

void RollerCoaster::setup()
{
    // Call the base first
    ApplicationContext::setup();
    addInputListener(this);

    // Get a pointer to the already created root
    Root* root = getRoot();

    // Scene Manager
    scnMgr = root->createSceneManager();
    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);
    scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    SceneNode* worldNode = scnMgr->getRootSceneNode()->createChildSceneNode("worldNode");

    // Light
    Light* light = scnMgr->createLight("MainLight");
    SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode("lightNode");
    lightNode->attachObject(light);
    lightNode->setPosition(10, 10, 10);

    // Camera
    SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode("camNode");
    Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    camNode->setPosition(0, 0, 50);
    camNode->lookAt(Ogre::Vector3(0, 0, -1), Node::TS_PARENT);
    
    // Add camera to viewport
    getRenderWindow()->addViewport(cam);
    scnMgr->addRenderQueueListener(getOverlaySystem());
    
    // TrayManager
    mTrayMgr = new TrayManager("InterfaceRCE", getRenderWindow(), this);
    addInputListener(mTrayMgr);
    mTrayMgr->hideCursor(); // Hide cursor of Ogre

    this->loadResource();
    this->menuGUI();
}

// END BASIC

// START GUI
void RollerCoaster::menuGUI()
{
    // Create background material
    MaterialPtr material = MaterialManager::getSingleton().create("BackgroundMenu", "General");
    material->getTechnique(0)->getPass(0)->createTextureUnitState("rail2.tga");
    material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
    material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    material->getTechnique(0)->getPass(0)->setLightingEnabled(false);

    // Create background rectangle covering the whole screen
    Rectangle2D* rect = new Rectangle2D(true);
    rect->setCorners(-1.0, 1.0, 1.0, -1.0);
    rect->setMaterial(material);

    // Render the background before everything else
    rect->setRenderQueueGroup(RENDER_QUEUE_BACKGROUND);

    // Use infinite AAB to always stay visible
    AxisAlignedBox aabInf;
    aabInf.setInfinite();
    rect->setBoundingBox(aabInf);

    // Attach background to the scene
    SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode("Background");
    node->attachObject(rect);

    // Background scrolling
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setScrollAnimation(0.0, 0.25);

    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.40;
    mTrayMgr->createButton(TL_CENTER, "PlayButton", "PLAY", buttonWidth);
    mTrayMgr->createButton(TL_CENTER, "SettingsButton", "SETTINGS", buttonWidth);
    mTrayMgr->createButton(TL_CENTER, "CreditsButton", "CREDITS", buttonWidth);
    mTrayMgr->createButton(TL_CENTER, "ExitButton", "EXIT", buttonWidth);
}

void RollerCoaster::creditsGUI()
{
    // Labels (Position, ID, Value)
    float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.80;
    Label* alejandro = mTrayMgr->createLabel(TL_CENTER, "alejandro", "Alejandro Mujica", labelWidth);
    Label* anthony = mTrayMgr->createLabel(TL_CENTER, "anthony", "Anthony Dugarte", labelWidth);
    Label* kevin = mTrayMgr->createLabel(TL_CENTER, "kevin", "Kevin Márquez", labelWidth);
    Label* lewis = mTrayMgr->createLabel(TL_CENTER, "lewis", "Lewis Ochoa", labelWidth);

    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.40;
    mTrayMgr->createButton(TL_CENTER, "ReturnButton", "RETURN", buttonWidth);
}

// END GUI

// START INTERFACE

// Override from TrayListener to manage click events in buttons
void RollerCoaster::buttonHit(Button * button)
{
    if(button->getCaption() == "PLAY")
        this->closeApp();
    if(button->getCaption() == "SETTINGS")
        this->closeApp();
    if(button->getCaption() == "CREDITS")
        this->creditsGUI();
    if(button->getCaption() == "RETURN")
        this->menuGUI();
    if(button->getCaption() == "EXIT")
        this->closeApp();
}

// END INTERFACE

// START TOOL

void RollerCoaster::loadResource()
{
    try
    {
        // Check the file extension
        if(resourcesFile.substr(resourcesFile.find_last_of(".") + 1) != "cfg")
            std::cerr << "Error: The file extension is not .cfg" << '\n';
        else
        {
            std::ifstream ifs(resourcesFile.c_str(), std::ios::binary|std::ios::in);
            // Check the file opening
            if (ifs.is_open())
            {
                Ogre::ConfigFile cf;
                cf.load(resourcesFile);
                Ogre::String name, locType;
                Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();
                while (secIt.hasMoreElements())
                {
                    Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
                    Ogre::ConfigFile::SettingsMultiMap::iterator it;
                    for (it = settings->begin(); it != settings->end(); ++it)
                    {
                        locType = it->first;
                        name = it->second;
                        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(name, locType);
                    }
                }
                Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup("General");
                Ogre::ResourceGroupManager::getSingletonPtr()->loadResourceGroup("General");
		        ifs.close();
            }
            else
                std::cerr << "Error: The Resource file could not be opened" << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred during load resources" << e.what() << '\n';
    }
}

// END TOOL

int main(int argc, char **argv)
{
    try
    {
    	RollerCoaster app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred during execution: " << e.what() << '\n';
        return 1;
    }
    return 0;
}