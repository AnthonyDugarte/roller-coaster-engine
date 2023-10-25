#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreRay.h"
#include "OgreSceneQuery.h"

#include <map>
#include <memory>
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
        std::pair<std::string, Vector3> check_intersect(SceneNode * node, Ray & ray);
        

    private:
        bool worldWasClicked;
        bool mMovableFound;
        const float rotationSpeed;
        const float moveSpeed;
        
        SceneManager* scnMgr;
        RaySceneQuery* mRayScnQuery;
        
    private:
        void rotateScene(int , int ) noexcept;
};

RollerCoaster::RollerCoaster() :
    ApplicationContext("OgreTutorialApp"),
    worldWasClicked{false},
    mMovableFound{false},
    rotationSpeed{0.5f},
    moveSpeed{5.f},
    scnMgr{nullptr},
    mRayScnQuery{0}
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

    Entity* mokeyEntity1 = scnMgr->createEntity("ogrehead.mesh");
    monkeyNode1->attachObject(mokeyEntity1);
    monkeyNode1->setPosition(0, 0, 0);
}

// Handle mouse wheel events (Zoom in or Zoom out)
bool RollerCoaster::mouseWheelRolled(const MouseWheelEvent &evt) // Zoom in/out
{    
    auto direction = scnMgr->getCamera("myCam")->getRealDirection();
    auto cameraNode = scnMgr->getSceneNode("camNode");
    cameraNode->translate(evt.y * direction * 0.5);

    return true;
}

// string: Name of node, Vector3: collision point 
std::pair<std::string, Vector3> RollerCoaster::check_intersect(SceneNode * node, Ray & ray)
{
    for (Node* child : node->getChildren())
    {
        SceneNode * scnNode = scnMgr->getSceneNode(child->getName());
        RayTestResult rTR = ray.intersects(scnNode->_getWorldAABB());
        if (rTR.first) 
        {
            return std::make_pair<std::string, Vector3>(std::string(child->getName()), ray.getPoint(rTR.second));
        }
    }

    return std::make_pair<String, Vector3>("", {});
}

// Handle click events (Save click position)
bool RollerCoaster::mousePressed(const MouseButtonEvent &evt)
{
    worldWasClicked = true;
    
    Camera* myCam {scnMgr->getCamera("myCam")};
    
    Ray mouseRay {myCam->getCameraToViewportRay(
            evt.x / float(myCam->getViewport()->getActualWidth()),
            evt.y / float(myCam->getViewport()->getActualHeight()))
        };

    auto result = check_intersect(scnMgr->getSceneNode("worldNode"), mouseRay);
    
    std::cout << result.first << " " << result.second << std::endl;
    
    mMovableFound = false;
    
    return true;
}	

// Handle realese events (Save realese position)
bool RollerCoaster::mouseReleased(const MouseButtonEvent &evt)
{
    worldWasClicked = false;
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
    else if (evt.keysym.sym == 119) // Key "w" : move up camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealDirection();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(moveSpeed * direction);
    }
    else if (evt.keysym.sym == 115) //Key "s" : move down camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealDirection();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(moveSpeed * (-direction));
    }
    
    else if (evt.keysym.sym == 97) // Key "a" : move left camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(-direction * moveSpeed);
    }
    else if (evt.keysym.sym == 100) // Key "d" : move right camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(direction * moveSpeed);
    }
    
    return true;
}

// Aply a rotation based on mouse speed
void RollerCoaster::rotateScene(int speedX, int speedY) noexcept
{
    auto cameraNode = scnMgr->getSceneNode("camNode");
    cameraNode->pitch(Degree(speedY * rotationSpeed));
    cameraNode->yaw(Degree(speedX * rotationSpeed));
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
