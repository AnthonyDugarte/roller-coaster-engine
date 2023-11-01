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
        RollerCoaster();
        virtual ~RollerCoaster() {}

        void setup();
        void menuGUI();
        void creditsGUI();
        void buttonHit(Button *	button);
        
    private:
        SceneManager* scnMgr;
        TrayManager* mTrayMgr;        
};

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

    menuGUI();
}

//START GUI
void RollerCoaster::menuGUI()
{
    //Load texture
    Ogre::ResourceGroupManager::getSingletonPtr()->createResourceGroup("UserDefinedMaterials");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("/home/lewis8a/roller-coaster-engine/project/build/texture", "FileSystem", "UserDefinedMaterials", true);
	Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup("UserDefinedMaterials");
	Ogre::ResourceGroupManager::getSingletonPtr()->loadResourceGroup("UserDefinedMaterials");

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
//END GUI

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