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
        void creditsGUI(int);

        //Interface
        void buttonHit(Button *	button);

        //Tool
        void loadResource();
        int randomNumber(int,int);
        
    private:
        SceneManager* scnMgr;
        TrayManager* trayMgr;        
};

// START BASIC
RollerCoaster::RollerCoaster() :
    ApplicationContext("Roller Coaster Engine"),
    scnMgr{nullptr},
    trayMgr{nullptr}
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
    trayMgr = new TrayManager("InterfaceRCE", getRenderWindow(), this);
    addInputListener(trayMgr);
    trayMgr->hideCursor(); // Hide cursor of Ogre

    this->loadResource();
    this->menuGUI();
}

// END BASIC

// START GUI
void RollerCoaster::menuGUI()
{
    // Clean
    this->trayMgr->destroyAllWidgets();

    Node * background  = 0;  // initialize outside the try/catch block
    try
	{
        // The child background exist
		background = scnMgr->getRootSceneNode()->getChild("Background"); 
	}
	catch (Ogre::Exception e)
	{
		// The child background does not exist
	
        // Create background material
        MaterialPtr material = MaterialManager::getSingleton().create("BackgroundMenu", "General");
        material->getTechnique(0)->getPass(0)->createTextureUnitState("rail"+std::to_string(this->randomNumber(1,4))+".jpg");
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
    }
    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.40;
    trayMgr->createButton(TL_CENTER, "PlayButton", "PLAY", buttonWidth);
    trayMgr->createButton(TL_CENTER, "SettingsButton", "SETTINGS", buttonWidth);
    trayMgr->createButton(TL_CENTER, "CreditsButton", "CREDITS", buttonWidth);
    trayMgr->createButton(TL_CENTER, "ExitButton", "EXIT", buttonWidth);
}

void RollerCoaster::creditsGUI(int part)
{
    if(part == 1)
    {
        // Clean
        this->trayMgr->destroyAllWidgets();

        // Labels (Position, ID, Value)
        float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.80;
        float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.30;
        Label* title = trayMgr->createLabel(TL_CENTER, "title", "CREDITS", labelWidth);
        
        //TextBox (Position, ID, caption, width, height
        TextBox* developers = trayMgr->createTextBox(TL_CENTER, "developers", "DEVELOPERS", labelWidth, labelHeight);
        // Set the body text
        developers->appendText("Alejandro Mujica\nAnthony Dugarte\nKevin Marquez");
        
        // Buttons (Position, ID, Value)
        float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        trayMgr->createButton(TL_CENTER, "NextButtonCredits", "NEXT", buttonWidth);
    }
    if(part == 2)
    {
        this->trayMgr->destroyWidget("NextButtonCredits");
        this->trayMgr->destroyWidget("developers");

        float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.80;
        float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.30;
        TextBox* copyright = trayMgr->createTextBox(TL_CENTER, "copyright", "ALKELEAN GAMES", labelWidth, labelHeight);

        //TextBox (Position, ID, caption, width, height
        copyright->appendText("Universidad de Los Andes\nFaculty Of Engineering\nComputer Graphics\nVenezuela 2023\n\nAll Rights Reserved");
        
        // Buttons (Position, ID, Value)
        float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        trayMgr->createButton(TL_CENTER, "ReturnButtonMain", "RETURN TO MAIN MENU", buttonWidth);
    }
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
        this->creditsGUI(1);
    if(button->getCaption() == "RETURN TO MAIN MENU")
        this->menuGUI();
    if(button->getCaption() == "EXIT")
        this->closeApp();
    if(button->getCaption() == "NEXT")
        this->creditsGUI(2);
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

int RollerCoaster::randomNumber(int low, int high)
{
    std::uniform_int_distribution<> dist (low,high);
    return dist(gen);
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