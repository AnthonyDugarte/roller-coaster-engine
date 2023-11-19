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
        // Basic
        RollerCoaster();
        virtual ~RollerCoaster() {}
        void setup();

        // GUI
        void menuGUI();
        void settingsGUI();
        void creditsGUI(int);
        void buildGUI();

        // Interface
        void play();
        void rotateCamera(int , int ) noexcept;
        void buttonHit(Button*);
        void sliderMoved(Slider*);
        void itemSelected(SelectMenu*);
        void windowResize(int,int);
        bool mouseWheelRolled(const MouseWheelEvent &);
        bool mousePressed(const MouseButtonEvent &);
        bool mouseReleased(const MouseButtonEvent &);
        bool mouseMoved(const MouseMotionEvent &);
        bool keyPressed(const KeyboardEvent &);
        void frameRendered(const Ogre::FrameEvent& evt);
        std::vector<std::pair<SceneNode*, Vector3>> get_intersections(SceneNode *, Ray &);

        // Tool
        void loadResource();
        int randomNumber(int,int);

    protected:
        // Terrain
        virtual void createScene();
        virtual void createFrameListener();
        virtual void destroyScene();
        //virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);

    private:
        // Terrain
        void getTerrainImage(bool, bool, Ogre::Image&);
        void defineTerrain(long, long);
        void initBlendMaps(Ogre::Terrain*);
        void configureTerrainDefaults(Ogre::Light*);

        // Interface
        void rotateScene(int , int ) noexcept;
        void resetHighlightedNode(void) noexcept;
        void setHighlightedNode(SceneNode*) noexcept;
        void get_intersections(SceneNode *, Ray &, std::map<Real, SceneNode*>&);
        void translateHighlightedNode(Vector3);
        void rotateHighlightedNode(float , bool);

        // Resource File
        std::string resourcesFile = "resources.cfg";
        
        // Basic
        SceneManager* scnMgr;
        TrayManager* trayMgr;
        RaySceneQuery* mRayScnQuery;
        int widthApp;
        int heightApp;
        int fxVolume;      
        int musicVolume;
        int sky;
        bool pause;
        Ogre::Timer timer;
        bool cameraMode;

        // Interface
        bool worldWasClicked;
        bool mMovableFound;
        const float rotationSpeed;
        std::pair<SceneNode*, Vector3> intersectedNode;
        SceneNode* highlightedNode;

        // Terrain
        bool mTerrainsImported;
        Ogre::TerrainGroup* mTerrainGroup;
        Ogre::TerrainGlobalOptions* mTerrainGlobals;
};

// START BASIC
RollerCoaster::RollerCoaster() :
    ApplicationContext("Roller Coaster Engine"),
    scnMgr{nullptr},
    trayMgr{nullptr},
    fxVolume{100},
    musicVolume{100},
    widthApp{800},
    heightApp{600},
    mTerrainGroup{0},
    mTerrainGlobals{0},
    worldWasClicked{false},
    mMovableFound{false},
    rotationSpeed{0.5f},
    highlightedNode{nullptr},
    mRayScnQuery{0},
    sky{1},
    pause{false},
    cameraMode{1}
{}

void RollerCoaster::setup()
{       
    //Hide Button Maximize
    Ogre::NameValuePairList parms;
    parms["border"] = "none";

    // Initialize root and create window
    mRoot->initialise(false);
    createWindow(mAppName,this->widthApp,this->heightApp,parms);

    // Locate and load resources
    locateResources();
    initialiseRTShaderSystem();
    loadResources();

    // Adds context as listener to process context-level (above the sample level) events
    mRoot->addFrameListener(this);
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
    lightNode->setPosition(0, 0, 0);

    // Camera
    SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode("camNode");
    Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5);
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    cam->setNearClipDistance(0.1);
    camNode->setPosition(Ogre::Vector3(1683, 50, 2116));
    camNode->lookAt(Ogre::Vector3(1963, 50, 1660), Node::TS_PARENT);

    // Check to see if our current render system has the capability to handle an infinite far clip distance. 
    //If it does, then we set the far clip distance to zero (which means no far clipping). 
    //If it does not, then we simply set the distance really high so we can see distant terrain. 
    bool infiniteClip = mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE);
    
    if (infiniteClip)
        cam->setFarClipDistance(0);
    else
        cam->setFarClipDistance(50000);
    
    // Add camera to viewport
    getRenderWindow()->addViewport(cam);
    scnMgr->addRenderQueueListener(getOverlaySystem());
    
    // TrayManager
    trayMgr = new TrayManager("InterfaceRCE", getRenderWindow(), this);
    addInputListener(trayMgr);
    trayMgr->hideCursor(); // Hide cursor of Ogre

    // Load sounds and resources
    Settings::loadSounds();
    this->loadResource();
    Settings::playMainMenuMusic();
    this->menuGUI();
}

// END BASIC

// START GUI
void RollerCoaster::menuGUI()
{
    // Clean
    this->trayMgr->destroyAllWidgets();

    SceneNode * background  = nullptr;
    if (!scnMgr->hasSceneNode("Background"))
    {
        this->sky = this->randomNumber(1,4);
		// The child background does not exist
	
        // Create background material
        MaterialPtr material = MaterialManager::getSingleton().create("BackgroundMenu", "General");
        material->getTechnique(0)->getPass(0)->createTextureUnitState("rail"+std::to_string(this->sky)+".jpg");
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
    else
    {    
        background = scnMgr->getSceneNode("Background");
    }
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "LogoRoller", "SdkTrays/LogoRoller"), TL_CENTER, 2000); // Show Logo of ROLLER COASTER
    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
    trayMgr->createButton(TL_CENTER, "PlayButton", "PLAY", buttonWidth);
    trayMgr->createButton(TL_CENTER, "SettingsButton", "SETTINGS", buttonWidth);
    trayMgr->createButton(TL_CENTER, "CreditsButton", "CREDITS", buttonWidth);
    trayMgr->createButton(TL_CENTER, "ExitButton", "EXIT", buttonWidth);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "LogoAlkelean", "SdkTrays/LogoAlkelean"), TL_BOTTOMRIGHT, 2000); // Show Logo of ALKELEAN GAMES
}

void RollerCoaster::settingsGUI()
{
    // Clean
    this->trayMgr->destroyAllWidgets();

    float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
    float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.50;
    
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "LogoRoller", "SdkTrays/LogoRoller"), TL_CENTER, 2000); // Show Logo of ROLLER COASTER
    
    // Put the caption beside selected item, width must be bigger than box width (Position, ID, Value, width, items, options)
    trayMgr->createThickSelectMenu(TL_CENTER, "resolution", "Resolution", labelWidth, 4, {"800x600", "1024x760", "1024x768", "1152x864", "1280x720", "1280x768", "1280x800", "1280x960", "1280x1024", "1360x764", "1400x1050", "1440x900", "1600x1200", "1680x1050", "1792x1344", "1856x1392", "1920x1080", "1920x1200", "1920x1440", "2560x1440", "2560x1600", "2880x1800", "3840x2160", "3840x2400"});
    // Slider (Position, ID, Title, widthText, widthValue, MinValue, MaxValue, Division+1)
    OgreBites::Slider *fx = trayMgr->createThickSlider(TL_CENTER, "effects", "fx Volume", labelWidth, 50, 0, 100, 101);
    OgreBites::Slider *music = trayMgr->createThickSlider(TL_CENTER, "music", "Music Volume", labelWidth, 50, 0, 100, 101);
    fx->setValue(this->fxVolume);
    music->setValue(this->musicVolume);
    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
    if(this->mTerrainsImported)
        trayMgr->createButton(TL_CENTER, "ReturnBuildButtonHide", "RETURN TO BUILD", buttonWidth);
    else
        trayMgr->createButton(TL_CENTER, "ReturnButtonMain", "RETURN TO MAIN MENU", buttonWidth);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "LogoAlkelean", "SdkTrays/LogoAlkelean"), TL_BOTTOMRIGHT, 2000); // Show Logo of ALKELEAN GAMES
}

void RollerCoaster::creditsGUI(int part)
{
    if(part == 1)
    {
        // Clean
        this->trayMgr->destroyAllWidgets();

        // Labels (Position, ID, Value)
        float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.50;
        Label* titleCredits = trayMgr->createLabel(TL_CENTER, "titleCredits", "CREDITS", labelWidth);
        
        //TextBox (Position, ID, caption, width, height
        TextBox* developers = trayMgr->createTextBox(TL_CENTER, "developers", "DEVELOPERS", labelWidth, labelHeight);
        // Set the body text
        developers->appendText("Alejandro Mujica\nAnthony Dugarte\nKevin Marquez\nLewis Ochoa");
        
        // Buttons (Position, ID, Value)
        float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        trayMgr->createButton(TL_CENTER, "NextButtonCredits", "NEXT", buttonWidth);
    }
    if(part == 2)
    {
        this->trayMgr->destroyWidget("NextButtonCredits");
        this->trayMgr->destroyWidget("developers");

        float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.50;
        TextBox* copyright = trayMgr->createTextBox(TL_CENTER, "copyright", "ALKELEAN GAMES", labelWidth, labelHeight);

        //TextBox (Position, ID, caption, width, height
        copyright->appendText("Universidad de Los Andes\nFaculty Of Engineering\nComputer Graphics\nVenezuela 2023\n\nAll Rights Reserved");
        
        // Buttons (Position, ID, Value)
        float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
        trayMgr->createButton(TL_CENTER, "ReturnButtonMain", "RETURN TO MAIN MENU", buttonWidth);
    }
    trayMgr->showLogo(TL_BOTTOM,2000); // Show Logo of ALKELEAN GAMES
}

void RollerCoaster::buildGUI()
{
    // Clean
    this->trayMgr->destroyAllWidgets();

    trayMgr->createLabel(TL_TOPLEFT, "coinLabel", "400", 100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Coin", "SdkTrays/Coin"), TL_TOPLEFT, 2000); // Show Icon Coin
    trayMgr->createLabel(TL_TOPLEFT, "clockLabel", "03:00", 100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Clock", "SdkTrays/Clock"), TL_TOPLEFT, 2000); // Show Icon Clock
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Open", "SdkTrays/Open"), TL_TOPLEFT, 2000); // Show Icon Open
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Rail", "SdkTrays/Rail"), TL_RIGHT, 2000); // Show Icon Rail
    trayMgr->createButton(TL_RIGHT, "NewRailButton", "New",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Destroy", "SdkTrays/Destroy"), TL_RIGHT, 2000); // Show Icon Destroy
    trayMgr->createButton(TL_RIGHT, "RemoveRailButton", "Remove",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Decoration", "SdkTrays/Decoration"), TL_RIGHT, 2000); // Show Icon Decoration
    trayMgr->createButton(TL_RIGHT, "DecorationButton", "Decor.",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Return", "SdkTrays/Return"), TL_RIGHT, 2000); // Show Icon Return
    trayMgr->createButton(TL_RIGHT, "UndoButton", "Undo",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Map", "SdkTrays/Map"), TL_RIGHT, 2000); // Show Icon Map
    trayMgr->createButton(TL_RIGHT, "MapButton", "Map",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Repair", "SdkTrays/Repair"), TL_RIGHT, 2000); // Show Icon Repair
    trayMgr->createButton(TL_RIGHT, "RepairButton", "Repair",100);
    trayMgr->moveWidgetToTray(trayMgr->createDecorWidget(TL_NONE, "Setting", "SdkTrays/Setting"), TL_BOTTOMLEFT, 2000); // Show Icon Setting
    trayMgr->createButton(TL_BOTTOMLEFT, "SettingButton", "Settings",130);
}

// END GUI

// START INTERFACE

void RollerCoaster::play()
{
    // Clean
    this->scnMgr->destroySceneNode("Background");
    this->trayMgr->destroyAllWidgets();

    //Skybox
    scnMgr->setSkyBox(true, "Sky/SkyBox"+std::to_string(this->sky));
    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);
    scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    // Monkey Suzanne
    SceneNode* ogreNode1 = scnMgr->getSceneNode("worldNode")->createChildSceneNode("ogreNode1");
    Entity* ogreEntity1 = scnMgr->createEntity("Suzanne.mesh");
    ogreNode1->attachObject(ogreEntity1);
    ogreNode1->setPosition(1683, 50, 2110);

    // Create terrain
    this->createScene();

    while(mTerrainGroup->isDerivedDataUpdateInProgress())
    {
        std::cout<<"Building terrain...\n";
        // we need to wait for this to finish
        OGRE_THREAD_SLEEP(1000);
        Root::getSingleton().getWorkQueue()->processResponses();
    }

    // Labels (Position, ID, Value)
    float labelWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
    float labelHeight = getRenderWindow()->getViewport(0)->getActualHeight() * 0.50;
    Label* titlePlay = trayMgr->createLabel(TL_CENTER, "titlePlay", "ROLLER COASTER ENGINE", labelWidth);
    
    //TextBox (Position, ID, caption, width, height
    TextBox* welcome = trayMgr->createTextBox(TL_CENTER, "welcome", "WELCOME", labelWidth, labelHeight);
    // Set the body text
    welcome->appendText("Welcome to the exciting world of roller coaster building! Get ready to design and build the most incredible and exciting roller coasters you have ever imagined. Defy gravity, create dizzying spins and thrilling drops as you build your own roller coaster. Have fun and enjoy the ride!");
    
    // Buttons (Position, ID, Value)
    float buttonWidth = getRenderWindow()->getViewport(0)->getActualWidth() * 0.60;
    trayMgr->createButton(TL_CENTER, "OkButtonPlay", "OK", buttonWidth);
    Settings::stopMainMenuMusic();
    Settings::playAmbienceMusic();
}  

// Aply a rotation based on mouse speed
void RollerCoaster::rotateCamera(int speedX, int speedY) noexcept
{
    auto cameraNode = scnMgr->getSceneNode("camNode");
    cameraNode->pitch(Degree(-speedY * 0.3), Node::TS_LOCAL);
    cameraNode->yaw(Degree(-speedX * 0.3), Node::TS_WORLD);
}

// Override from TrayListener to manage click events in buttons
void RollerCoaster::buttonHit(Button * button)
{
    Settings::sounds["click"].play();
    if(button->getCaption() == "PLAY")
    {
        this->play();
        Settings::sounds["start"].play();
    }
    if(button->getCaption() == "SETTINGS" || button->getCaption() == "Settings")
    {
        this->pause = true;
        this->settingsGUI();
    }
    if(button->getCaption() == "CREDITS")
        this->creditsGUI(1);
    if(button->getCaption() == "RETURN TO MAIN MENU")
        this->menuGUI();
    if(button->getCaption() == "EXIT")
    {
        delete this->trayMgr;
        getRoot()->queueEndRendering();
    }
    if(button->getCaption() == "NEXT")
        this->creditsGUI(2);
     if(button->getCaption() == "OK" || button->getCaption() == "RETURN TO BUILD")
     {
        this->pause = false;
        this->buildGUI();
    }
}

// Override from TrayListener to manage slide events
void RollerCoaster::sliderMoved(Slider * slider)
{
    if (slider->getName().compare("effects") == 0)
    {
        Settings::setFxVolume(this->fxVolume);
        this->fxVolume = slider->getValue();
    }
    else
    {
        this->musicVolume = slider->getValue();
        Settings::setMusicVolume(this->musicVolume);
    }
    Settings::sounds["slider"].play();
}

// Override from TrayListener to manage slide events
void RollerCoaster::itemSelected(SelectMenu *menu)
{
    if (menu->getName().compare("resolution") == 0)
    {
        std::string resolution = menu->getSelectedItem();
        this->widthApp = std::stoi(resolution.substr(0,resolution.find_last_of("x")));
        this->heightApp = std::stoi(resolution.substr(resolution.find_last_of("x") + 1));
        this->windowResize(widthApp,heightApp);
    }
}

void RollerCoaster::windowResize(int width, int height)
{
    getRenderWindow()->resize(width,height);
    // Set the aspect ratio for the new size
    scnMgr->getCamera("myCam")->setAspectRatio(width / height);
    // Letting Ogre know the window has been resized
    this->windowResized(getRenderWindow());
    //getRenderWindow()->windowMovedOrResized();
}

// Handle mouse wheel events (Zoom in or Zoom out)
bool RollerCoaster::mouseWheelRolled(const MouseWheelEvent &evt) // Zoom in/out
{
    if(this->mTerrainsImported && !pause)
    {
        auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
        auto cameraNode {scnMgr->getSceneNode("camNode")};
        cameraNode->translate(evt.y * direction * 1.5);
        return true;
    }
    return false;
}

// Handle click events (Save click position)
bool RollerCoaster::mousePressed(const MouseButtonEvent &evt)
{
    resetHighlightedNode();
    Camera* myCam {scnMgr->getCamera("myCam")};
    
    Ray mouseRay {
        myCam->getCameraToViewportRay(
            evt.x / float(myCam->getViewport()->getActualWidth()),
            evt.y / float(myCam->getViewport()->getActualHeight())
            )
        };

    std::vector<std::pair<SceneNode*, Vector3>> result {get_intersections(scnMgr->getSceneNode("worldNode"), mouseRay)};
    if (not result.empty()) // Have colisions
    {
        setHighlightedNode(result[0].first);
        mMovableFound = true; // Flag to mark a selection
    }
    else // Click over world
    {
        worldWasClicked = true;
    }   
    
    return true;
}

// Handle realese events (Save realese position)
bool RollerCoaster::mouseReleased(const MouseButtonEvent &evt)
{   
    if(!cameraMode)
        resetHighlightedNode();
    worldWasClicked = false;
    return true;
}	

// Handle mouse movement events (Rotation direction, or move a node)
bool RollerCoaster::mouseMoved(const MouseMotionEvent &evt)
{    
    if(this->mTerrainsImported && !pause)
    {
        // evt: type, windowID, x, y, xrel, yrel
        Camera* myCam {scnMgr->getCamera("myCam")};
        
        Ray mouseRay {
            myCam->getCameraToViewportRay(
                evt.x / float(myCam->getViewport()->getActualWidth()),
                evt.y / float(myCam->getViewport()->getActualHeight())
                )
            };

        if (!pause) // Rotate scene
        {    
            if(cameraMode)
                if(worldWasClicked)
                    rotateScene(evt.xrel, evt.yrel); // rotate based on relative mouse speed
            else
                rotateCamera(evt.xrel, evt.yrel); // rotate based on relative mouse speed
        }
        return true;
    }
    return false;
}

// Handle keyboard events (Translate or rotate camera)
bool RollerCoaster::keyPressed(const KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE) // Press Esc
    {
        getRoot()->queueEndRendering();
    }
    else if (evt.keysym.sym == SDLK_UP) // Up arrow : rotate x+ camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        if (highlightedNode == nullptr)
            cameraNode->pitch(Degree(5), Node::TS_LOCAL);
        else
            rotateHighlightedNode(90, true);
    }
    else if (evt.keysym.sym == SDLK_DOWN) // Down arrow : rotate x- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        if (highlightedNode == nullptr)
            cameraNode->pitch(Degree(-5), Node::TS_LOCAL);
        else
            rotateHighlightedNode(-90, true);
    }
    else if (evt.keysym.sym == SDLK_LEFT) // Left arrow : rotate y- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        if (highlightedNode == nullptr)
            cameraNode->yaw(Degree(5), Node::TS_WORLD);
        else
            rotateHighlightedNode(90, false);
    }
    else if (evt.keysym.sym == SDLK_RIGHT) // Right arrow : rotate y+ camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        if (highlightedNode == nullptr)
            cameraNode->yaw(Degree(-5), Node::TS_WORLD);
        else
            rotateHighlightedNode(-90, false);
    }
    else if (evt.keysym.sym == 119) // Key "w" : move up camera
    {
        auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
        auto cameraNode {scnMgr->getSceneNode("camNode")};
        cameraNode->translate(direction * 1.5);
        translateHighlightedNode(direction);
    }
    else if (evt.keysym.sym == 115) //Key "s" : move down camera
    {
        auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
        auto cameraNode {scnMgr->getSceneNode("camNode")};
        cameraNode->translate(-direction * 1.5);
        translateHighlightedNode(-direction);
    }
    
    else if (evt.keysym.sym == 97) // Key "a" : move left camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(-direction * 1.5);
        translateHighlightedNode(-direction);
    }
    else if (evt.keysym.sym == 100) // Key "d" : move right camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(direction * 1.5);
        translateHighlightedNode(direction);
    }
    else if (evt.keysym.sym == 99) // Key "c" : change mode camera
    {
        resetHighlightedNode();
        if(cameraMode)
            cameraMode = 0;
        else
            cameraMode = 1;
    }
    
    return true;
}

void RollerCoaster::frameRendered(const Ogre::FrameEvent&)
{
    if(mTerrainsImported && this->timer.getMilliseconds() > 288000)
    {
        if(this->sky < 5)
            this->sky++;
        else
            this->sky = 1;
            scnMgr->setSkyBox(true, "Sky/SkyBox"+std::to_string(this->sky));
        timer.reset();
    } 
}

void RollerCoaster::setHighlightedNode(SceneNode* node) noexcept
{
    if (highlightedNode == nullptr)
    {
        highlightedNode = node;
        highlightedNode->showBoundingBox(true);
    }
    else if (highlightedNode != node)
    {
        resetHighlightedNode();
        setHighlightedNode(node);
    }
}

// Apply a rotation based on mouse speed
void RollerCoaster::rotateScene(int speedX, int speedY) noexcept
{
    // Moving camNode to TempNode
    SceneNode* camNode {scnMgr->getSceneNode("camNode")};
    SceneNode* fatherCamera {camNode->getParentSceneNode()};
    if(abs(speedX) > abs(speedY))
        camNode->yaw(Degree(speedX * 0.1));
    else
        camNode->pitch(Degree(speedY * 0.1));
}

void RollerCoaster::resetHighlightedNode(void) noexcept
{
    if (highlightedNode != nullptr)
    {    
        highlightedNode->showBoundingBox(false);
        highlightedNode = nullptr;
    }
}

// wrapper to recursive get_intersections
// SceneNode: Node intersected, Vector3: collision point 
std::vector<std::pair<SceneNode*, Vector3>> RollerCoaster::get_intersections(SceneNode * node, Ray & ray)
{
    std::map<Real, SceneNode*> matchs; // Intersections ordered by distance
    std::vector<std::pair<SceneNode*, Vector3>> intersections; // Intersections ordered by distance with point intersection
    
    get_intersections(node, ray, matchs); // Get all matchs
    // Transform map<Distance, SceneNode*> to vector<pair<SceneNode*, Vector3>>
    for (std::pair<Real, SceneNode*> match : matchs)
    {
        intersections.emplace_back(match.second, ray.getPoint(match.first));
    }
    
    return intersections;
}

void RollerCoaster::get_intersections(SceneNode* node, Ray& ray, std::map<Real, SceneNode*>& matchs)
{
    std::vector<Node*> childs {node->getChildren()};
    // Deep First Search
    if (childs.size() > 0) // Not a leave
    {
        for (Node* child : childs)
        {
            SceneNode* n {scnMgr->getSceneNode(child->getName())};
            get_intersections(n, ray, matchs);
        }
    }
    else // Find a leave
    {
        SceneNode* camNode {scnMgr->getSceneNode("camNode")};
        RayTestResult rTR {ray.intersects(node->_getWorldAABB())};
        // Generate a map<Distance, SceneNode*> with intersections ordered by distance
        if (rTR.first and camNode != node) // Intersection
        {
            matchs.emplace(rTR.second, node);
        }
    }
}

void RollerCoaster::translateHighlightedNode(Vector3 direction)
{
    if (highlightedNode != nullptr)
        highlightedNode->translate(direction * 1.5);
}

void RollerCoaster::rotateHighlightedNode(float angle, bool pitch)
{
    if (highlightedNode != nullptr)
    {
        SceneNode* camNode {scnMgr->getSceneNode("camNode")};
        Vector3 nodePosition = highlightedNode->getPosition();
        Vector3 cameraPosition = camNode->getPosition();
        Vector3 direction = (nodePosition - cameraPosition).normalisedCopy();
        Vector3 yAxis = Vector3::UNIT_Y;
        Vector3 xAxis = scnMgr->getCamera("myCam")->getRealRight();
        Vector3 zAxis = scnMgr->getCamera("myCam")->getRealUp();
        if(pitch)
            highlightedNode->rotate(xAxis, Radian(-angle),Node::TS_PARENT);
        else
            highlightedNode->rotate(yAxis, Radian(-angle),Node::TS_PARENT);
    }
}
/*
// Find all intersected nodes


*/
// END INTERFACE

// START TOOL

void RollerCoaster::loadResource()
{
    try
    {
        // Check the file extension
        if(this->resourcesFile.substr(this->resourcesFile.find_last_of(".") + 1) != "cfg")
            std::cerr << "Error: The file extension is not .cfg" << '\n';
        else
        {
            std::ifstream ifs(this->resourcesFile.c_str(), std::ios::binary|std::ios::in);
            // Check the file opening
            if (ifs.is_open())
            {
                Ogre::ConfigFile cf;
                cf.load(this->resourcesFile);
                Ogre::String name, locType;
                for (const auto& ti : cf.getSettings())
                {
                    locType = ti.first; 
                    name = ti.second;
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(name, locType);
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

// START TERRAIN

void RollerCoaster::createScene()
{
    // Setting Up a Light for Our Terrain
    scnMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
    
    // Directional Light
    Ogre::Light* light = scnMgr->createLight("TestLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));

    // The normalise method will make the vector's length equal to one while maintaining its direction
    Ogre::SceneNode* ln = scnMgr->getRootSceneNode()->createChildSceneNode();
    ln->setDirection(Vector3(0.55, -0.3, 0.75).normalisedCopy());
    ln->attachObject(light);

    // This is a class that holds information for all of the terrains we might create 
    mTerrainGlobals = new Ogre::TerrainGlobalOptions();

    // The TerrainGroup constructor takes the SceneManager as its first parameter 
    // It then takes an alignment option, terrain size, and terrain world size
    // The setFilenameConvention allows us to choose how our terrain will be saved
    // Finally, we set the origin to be used for our terrain
    mTerrainGroup = new Ogre::TerrainGroup(scnMgr, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0);
    mTerrainGroup->setFilenameConvention(Ogre::String("terrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

    this->configureTerrainDefaults(light);

    for(long x = 0; x <= 0; ++x)
        for(long y = 0; y <= 0; ++y)
            defineTerrain(x, y);
    // Define our terrains and ask the TerrainGroup to load them all
    mTerrainGroup->loadAllTerrains(true);

    // Initialize the blend maps for our terrain
    if (mTerrainsImported)
    {
        for (const auto& ti : mTerrainGroup->getTerrainSlots())
        {
            initBlendMaps(ti.second->instance);
        }
    }
    // Cleanup any temporary resources that were created while configuring our terrain
    mTerrainGroup->freeTemporaryResources();
}

void RollerCoaster::createFrameListener()
{
}

// We must make sure to call OGRE_DELETE for every time we called OGRE_NEW
void RollerCoaster::destroyScene()
{
    delete mTerrainGroup;
    delete mTerrainGlobals;
}

/*
bool RollerCoaster::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
    if (mTerrainGroup->isDerivedDataUpdateInProgress())
    {
        if (mTerrainsImported)
            std::cout<<"Building terrain...\n";
        else
            std::cout<<"Updating textures, patience...\n";
    }
    else
    {
        if (mTerrainsImported)
        {
            // FIXME does not end up in the correct resource group
            // saveTerrains(true);
            mTerrainsImported = false;
        }
    }
    return 1;
}
*/

void RollerCoaster::getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
    // This will load our 'terrain.png' resource.
    img.load("terrain"+std::to_string(this->sky)+".png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
   
    // Flipping is used to create seamless terrain so that unlimited terrain can be created using a single heightmap
    if (flipX)
        img.flipAroundY();
    if (flipY)
        img.flipAroundX();
}

void RollerCoaster::defineTerrain(long x, long y)
{
    // Ask the TerrainGroup to define a unique filename for this Terrain.
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);
    // Check to see if a filename for this grid location has already been generated
    bool exists = Ogre::ResourceGroupManager::getSingleton().resourceExists( mTerrainGroup->getResourceGroup(), filename);
    
    //If it has already been generated, then we can call TerrainGroup::defineTerrain method to set up this grid location with the previously generated filename automatically. 
    if (exists)
        mTerrainGroup->defineTerrain(x, y);
    else //If it has not been generated, then we generate an image with getTerrainImage and then call a different overload of TerrainGroup::defineTerrain that takes a reference to our generated image. Finally, we set the mTerrainsImported flag to true. 
    {
        Ogre::Image img;
        getTerrainImage(x % 2 != 0, y % 2 != 0, img);
        mTerrainGroup->defineTerrain(x, y, &img);
        timer.reset(); //For the skybox
        mTerrainsImported = true;
    }
}

// This method will blend together the different layers we defined in configureTerrainDefaults
void RollerCoaster::initBlendMaps(Ogre::Terrain* terrain)
{
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    float* pBlend0 = blendMap0->getBlendPointer();

    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;
            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val;
        }
    }

    blendMap0->dirty();
    blendMap0->update();
}

void RollerCoaster::configureTerrainDefaults(Ogre::Light* light)
{
    // sets the largest error in pixels allowed between our ideal terrain and the mesh that is created to render it. 
    // A smaller number will mean a more accurate terrain, because it will require more vertices to reduce the error
    mTerrainGlobals->setMaxPixelError(8);
    
    // determines the distance at which Ogre will still apply our lightmap. 
    // If you increase this, then you will see Ogre apply lighting effects out to a farther distance. 
    mTerrainGlobals->setCompositeMapDistance(2000);

    // Pass our lighting information to our terrain
    mTerrainGlobals->setLightMapDirection(light->getDerivedDirection()); // Apply any transforms that are applied to our Light's direction by any SceneNode it may be attached to
    mTerrainGlobals->setCompositeMapAmbient(scnMgr->getAmbientLight()); // Set the ambient light
    mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour()); // Diffuse color for our terrain to match our scene lighting

    // Get a reference to the import settings of our TerrainGroup and set some basic values
    Ogre::Terrain::ImportData& importData = mTerrainGroup->getDefaultImportSettings();
    importData.terrainSize = 1024; //pixel of terrain.png
    importData.worldSize = 120.0;
    importData.inputScale = 0; //Montain
    importData.minBatchSize = 33; //2^n+1
    importData.maxBatchSize = 65; //2^n+1

    // This way you save storage space and speed up loading
    // However if you want more flexibility, you can also make Ogre combine the images at loading accordingly as shown below
    Image combined;
    combined.loadTwoImagesAsRGBA("Ground"+std::to_string(this->sky)+"_col.jpg", "Ground"+std::to_string(this->sky)+"_spec.png", "General");
    TextureManager::getSingleton().loadImage("Ground_diffspec", "General", combined);

    // Texture
    // The texture's worldSize determines how big each splat of texture is going to be when applied to the terrain. 
    // A smaller value will increase the resolution of the rendered texture layer because each piece will be stretched less to fill in the terrain. 
    importData.layerList.resize(2);
    importData.layerList[0].worldSize = 20;
    importData.layerList[0].textureNames.push_back("Ground_diffspec");
    importData.layerList[0].textureNames.push_back("Ground"+std::to_string(this->sky)+"_spec.png");
    importData.layerList[1].worldSize = 0;
    importData.layerList[1].textureNames.push_back("Ground_diffspec");
    importData.layerList[1].textureNames.push_back("Ground_normheight.dds");
}

// END TERRAIN

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
