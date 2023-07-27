#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreTrays.h"

#include <iostream>

using namespace Ogre;
using namespace OgreBites;

class RollerCoaster :
    public ApplicationContext,
    public InputListener, // Needed to manage events mouse, keyboard, ...
    public TrayListener // Needed to manage events in trays (buttons, layers, sliders...)
{
    public:
        RollerCoaster();
        virtual ~RollerCoaster() {}

        void setup();
        bool keyPressed(const KeyboardEvent &);
        void labelHit(Label *label);
        void buttonHit(Button *	button);
        void sliderMoved(Slider * slider);
        void checkBoxToggled(CheckBox *box);
        
    private:
        SceneManager* scnMgr;
        TrayManager* mTrayMgr;
        int countChecked {0};
        
};

RollerCoaster::RollerCoaster() :
    ApplicationContext("OgreTutorialApp"),
    scnMgr{nullptr},
    mTrayMgr{nullptr}
{
}

void RollerCoaster::setup()
{
    // do not forget to call the base first
    ApplicationContext::setup();
    addInputListener(this);

    // get a pointer to the already created root
    Root* root = getRoot();

    scnMgr = root->createSceneManager();

    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);
    scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    SceneNode* worldNode = scnMgr->getRootSceneNode()->createChildSceneNode("worldNode");

    //! [newlight]
    Light* light = scnMgr->createLight("MainLight");
    SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode("lightNode");
    lightNode->attachObject(light);

    //! [lightpos]
    lightNode->setPosition(10, 10, 10);

    //! [camera]
    SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode("camNode");

    // create the camera
    Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5); // specific to this sample
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    camNode->setPosition(0, 0, 50);
    camNode->lookAt(Ogre::Vector3(0, 0, -1), Node::TS_PARENT);
    
    //add camera to viewport
    getRenderWindow()->addViewport(cam);
       
//    // [Monkeys]
//    SceneNode* monkeyNode1 = worldNode->createChildSceneNode("monkeyNode1");
//    Entity* mokeyEntity1 = scnMgr->createEntity("Suzanne.mesh");
//    monkeyNode1->attachObject(mokeyEntity1);
//    monkeyNode1->setPosition(0, 0, 0);

    // GUI
    scnMgr->addRenderQueueListener(getOverlaySystem());
    
    // TrayManager
    mTrayMgr = new TrayManager("InterfaceName", getRenderWindow(), this);
    addInputListener(mTrayMgr);
    
    mTrayMgr->hideCursor(); // Hide cursor of Ogre
    
    // Widgets
    mTrayMgr->createButton(TL_TOPLEFT, "MyButton", "Click Me!"); //Position, ID, Value
    
    //Position, ID, caption, ancho, alto
    TextBox* myTextBox = mTrayMgr->createTextBox(TL_TOPRIGHT, "MyTextBox", "HOLA MUNDO!", 200, 100);    
    myTextBox->appendText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam quis tellus urna. Phasellus porttitor purus dolor, id dictum augue rutrum."); // set the body text
    // myTextBox->setPadding(); // Between words
    
    // Put the caption beside selected item, width must be bigger than box width
    SelectMenu* sLM = mTrayMgr->createLongSelectMenu(TL_TOP, "LongSM", "Long Select Menu", 300, 100, 4, {"Uno", "Dos", "Tres", "Cuatro", "Cinco"});
    if (sLM->containsItem("Cinco"))
    {
        sLM->addItem("Seis");
    }
    sLM->insertItem(0, "Cero");
    sLM->selectItem(3);
    
    // Put the caption over selected item
    SelectMenu* sTM = mTrayMgr->createThickSelectMenu(TL_TOP, "ThickSM", "Thick Select Menu", 200, 4, {"Uno", "Dos", "Tres", "Cuatro", "Cinco"});
    sTM->selectItem ("Tres");
    sTM->setItems({"Cinco", "Cuatro", "Tres", "Dos", "Uno"}); // Replaces exiting items, also flush selected item
    
    // Labels lb without width lb2 with width
    Label* lb = mTrayMgr->createLabel(TL_BOTTOM, "fLabel", "This is a label", 200);
    Label* lb2 = mTrayMgr->createLabel(TL_BOTTOMLEFT, "sLabel", "This is a label with width", 300);
    
    Separator* sp = mTrayMgr->createSeparator(TL_TOP,"fSeparator");
    
    mTrayMgr->createThickSlider(TL_TOP, "thickSlider", "Zoom", 200, 60, 0, 100, 101);
    mTrayMgr->createLongSlider(TL_TOP, "longSlider", "Zoom", 100, 50, 0, 100, 11);
    
    // Empty panel with 5 lines, can also be defined by a vector of strings
    // instead of an integer to indicate lines number
    ParamsPanel* fPanel = mTrayMgr->createParamsPanel(TL_BOTTOMRIGHT, "fPanel", 200, 5);
    fPanel->setAllParamNames({"Param 1", "Param 2", "Param 3", "Param 4", "Param 5"});
    fPanel->setParamValue(1, "1");
    fPanel->setParamValue("Param 5", " Five");
    
    // Fit to content
    CheckBox* cBox1 = mTrayMgr->createCheckBox(TL_BOTTOMRIGHT, "cBox1", "No, active me!");
    // Fixed width
    CheckBox* cBox2 = mTrayMgr->createCheckBox(TL_BOTTOMRIGHT, "cBox2", "Active me!", 150);
    
    ProgressBar* pB1 = mTrayMgr->createProgressBar(TL_CENTER,"pB1", "Loading", 250, 150);
}

// Handle keyboard events (Translate or rotate camera)
bool RollerCoaster::keyPressed(const KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE) // Press Esc
    {
        delete mTrayMgr;
        getRoot()->queueEndRendering();
    }
    
    return true;
}

// Override from TrayListener to manage click events in labels
void RollerCoaster::labelHit(Label *label)
{
    label->setCaption("Was clicked it");
}

// Override from TrayListener to manage click events in buttons
void RollerCoaster::buttonHit(Button * button)
{
    ProgressBar* pB1 = reinterpret_cast<ProgressBar *>(mTrayMgr->getWidget("pB1"));
    Real progress = pB1->getProgress() + 0.05; 
    std::cout << progress << std::endl;
    if (progress < 0.11f)
    {
        pB1->setComment("Hurry up!");
    }
    else if (progress < .31f)
    {
        pB1->setComment("Slowly!");
    }
    else if (progress < .51f)
    {
        pB1->setComment("Almost there!");
    }
    else if (progress < .99f)
    {
        pB1->setComment("ZzZzZzzz!");
    }
    else
    {
        pB1->setComment("Easy boy!");
    }
    
    pB1->setProgress(progress);
}

// Override from TrayListener to manage slide events
void RollerCoaster::sliderMoved(Slider * slider)
{
    ParamsPanel* fPanel = reinterpret_cast<ParamsPanel *>(mTrayMgr->getWidget("fPanel"));

    if (slider->getName().compare("thickSlider") == 0)
    {
        fPanel->setParamValue("Param 1", slider->getValueCaption());
    }
    else
    {
        fPanel->setParamValue("Param 3", slider->getValueCaption());
    }
}

// Override from TrayListener to manage slide events
void RollerCoaster::checkBoxToggled(CheckBox *box)
{
    CheckBox* cBox = nullptr;
    
    if (box->getName().compare("cBox1") == 0)
    {
        cBox = reinterpret_cast<CheckBox *>(mTrayMgr->getWidget("cBox2"));
    }
    else
    {
        cBox = reinterpret_cast<CheckBox *>(mTrayMgr->getWidget("cBox1"));
    }
    
    cBox->toggle(false);   
    box->toggle(false);
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
