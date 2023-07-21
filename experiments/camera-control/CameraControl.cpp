#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"

#include <iostream>

using namespace Ogre;
using namespace OgreBites;

class RollerCoaster : public ApplicationContext, public InputListener
{
    public:
        RollerCoaster();
        virtual ~RollerCoaster() {}

        void setup();
        bool mouseWheelRolled(const MouseWheelEvent &);
        bool mousePressed(const MouseButtonEvent &);
        bool mouseReleased(const MouseButtonEvent &);
        bool mouseMoved(const MouseMotionEvent &evt);
        bool keyPressed(const KeyboardEvent &);

    private:
        bool worldWasClicked;
        std::pair<int,int> clickPosition;
        SceneManager* scnMgr;
        const float rotationSpeed;
        
    private:
        void rotateScene(int , int ) noexcept;
};

RollerCoaster::RollerCoaster() :
    ApplicationContext("OgreTutorialApp"),
    worldWasClicked{false},
    clickPosition{-1, -1},
    scnMgr{nullptr},
    rotationSpeed{0.5f}
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
       
    // [Monkeys]
    SceneNode* monkeyNode1 = worldNode->createChildSceneNode("monkeyNode1");
    Entity* mokeyEntity1 = scnMgr->createEntity("Suzanne.mesh");
    monkeyNode1->attachObject(mokeyEntity1);
    monkeyNode1->setPosition(0, 0, 0);
    
    SceneNode* monkeyNode2 = worldNode->createChildSceneNode("monkeyNode2");
    Entity* mokeyEntity2 = scnMgr->createEntity("Suzanne.mesh");
    monkeyNode2->attachObject(mokeyEntity2);
    monkeyNode2->setPosition(25, 0, 0);
    
    SceneNode* monkeyNode3 = worldNode->createChildSceneNode("monkeyNode3");
    Entity* mokeyEntity3 = scnMgr->createEntity("Suzanne.mesh");
    monkeyNode3->attachObject(mokeyEntity3);
    monkeyNode3->setPosition(-25, 0, 0);
    
    SceneNode* monkeyNode4 = worldNode->createChildSceneNode("monkeyNode4");
    Entity* mokeyEntity4 = scnMgr->createEntity("Suzanne.mesh");
    monkeyNode4->attachObject(mokeyEntity4);
    monkeyNode4->setPosition(0, 25, 0);
    
    SceneNode* monkeyNode5 = worldNode->createChildSceneNode("monkeyNode5");
    Entity* mokeyEntity5 = scnMgr->createEntity("Suzanne.mesh");
    monkeyNode5->attachObject(mokeyEntity5);
    monkeyNode5->setPosition(0, -25, 0);
}

// Handle mouse wheel events (Zoom in or Zoom out)
bool RollerCoaster::mouseWheelRolled(const MouseWheelEvent &evt) // Zoom in/out
{    
    auto direction = scnMgr->getCamera("myCam")->getRealDirection();
    auto cameraNode = scnMgr->getSceneNode("camNode");
    cameraNode->translate(evt.y * direction * 0.5);

    return true;
}

// Handle click events (Save click position)
bool RollerCoaster::mousePressed(const MouseButtonEvent &evt)
{
    worldWasClicked = true;
    clickPosition.first = evt.x;
    clickPosition.second = evt.y;
    return true;
}	

// Handle realese events (Save realese position)
bool RollerCoaster::mouseReleased(const MouseButtonEvent &evt)
{
    worldWasClicked = false;
    clickPosition.first = evt.x;
    clickPosition.second = evt.y;
    return true;
}	

// Handle mouse movement events (Rotation direction)
bool RollerCoaster::mouseMoved(const MouseMotionEvent &evt)
{
    // evt: type, windowID, x, y, xrel, yrel
    if (worldWasClicked)
    {    
        rotateScene(evt.xrel, evt.yrel); // rotate based on relative mouse speed
    }
    return true;
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
        cameraNode->pitch(Degree(1));
    }
    else if (evt.keysym.sym == SDLK_DOWN) // Down arrow : rotate x- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->pitch(Degree(-1));
    }
    else if (evt.keysym.sym == SDLK_LEFT) // Left arrow : rotate y- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->yaw(Degree(-1));
    }
    else if (evt.keysym.sym == SDLK_RIGHT) // Right arrow : rotate y+ camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->yaw(Degree(1));
    }
    else if (evt.keysym.sym == 119) // Key "w" : move up camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealUp();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(direction * 0.5);
    }
    else if (evt.keysym.sym == 115) //Key "s" : move down camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealUp();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(-direction * 0.5);
    }
    
    else if (evt.keysym.sym == 97) // Key "a" : move left camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(-direction * 0.5);
    }
    else if (evt.keysym.sym == 100) // Key "d" : move right camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(direction * 0.5);
    }
    
    return true;
}

// Aply a rotation based on mouse speed
void RollerCoaster::rotateScene(int speedX, int speedY) noexcept
{
    auto world = scnMgr->getSceneNode("worldNode");
    world->yaw(Degree(speedX * rotationSpeed));
    world->roll(Degree(speedY * rotationSpeed));
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
