#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgrePredefinedControllers.h"
#include "OgreRTShaderSystem.h"
#include "OgreRay.h"
#include "OgreSceneQuery.h"
#include "OgreTrays.h"

#include <map>
#include <memory>
#include <iostream>

using namespace Ogre;
using namespace OgreBites;

enum class MovementAxis { x, y, z, none };

class RollerCoaster :
    public ApplicationContext,
    public InputListener,
    public TrayListener
{
    public:
        RollerCoaster();
        virtual ~RollerCoaster() {}

        void setup();
        bool mouseWheelRolled(const MouseWheelEvent &);
        bool mousePressed(const MouseButtonEvent &);
        bool mouseReleased(const MouseButtonEvent &);
        bool mouseMoved(const MouseMotionEvent &);
        bool keyPressed(const KeyboardEvent &);
        std::vector<std::pair<SceneNode*, Vector3>> get_intersections(SceneNode *, Ray &);
        void buttonHit(Button *);
        
    private:
        bool worldWasClicked;
        bool mMovableFound;
        const float rotationSpeed;
        MovementAxis moveAxis;
        
        std::pair<SceneNode*, Vector3> intersectedNode;
        SceneNode* highlightedNode;
        SceneManager* scnMgr;
        RaySceneQuery* mRayScnQuery;
        TrayManager* mTrayMgr;
        
    private:
        void rotateScene(int , int ) noexcept;
        void resetHighlightedNode(void) noexcept;
        void setHighlightedNode(SceneNode* node) noexcept;
        void get_intersections(SceneNode *, Ray &, std::map<Real, SceneNode*>&);
};

RollerCoaster::RollerCoaster() :
    ApplicationContext("OgreTutorialApp"),
    worldWasClicked{false},
    mMovableFound{false},
    rotationSpeed{0.5f},
    intersectedNode{std::make_pair<SceneNode*, Vector3>(nullptr, Vector3())},
    moveAxis{MovementAxis::none},
    highlightedNode{nullptr},
    scnMgr{nullptr},
    mTrayMgr{nullptr},
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

    //! [skybox]
    scnMgr->setSkyBox(true, "Examples/MorningSkyBox");

    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);
    scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    SceneNode* worldNode = scnMgr->getRootSceneNode()->createChildSceneNode("worldNode");
    //worldNode->setFixedYawAxis(true, Vector3(0, 1, 0));

    //! [newlight]
    Light* light = scnMgr->createLight("MainLight");
    SceneNode* lightNode = worldNode->createChildSceneNode("lightNode");
    lightNode->attachObject(light);

    //! [lightpos]
    lightNode->setPosition(10, 10, 10);

    //! [camera]
    SceneNode* camNode = worldNode->createChildSceneNode("camNode");

    // create the camera
    Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(50); // specific to this sample
    cam->setFarClipDistance(0);
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    camNode->setPosition(0, 0, 50);
    camNode->lookAt(Ogre::Vector3(0, 0, -1), Node::TS_PARENT);
    
    //add camera to viewport
    getRenderWindow()->addViewport(cam);
       
    // [Monkeys]
    SceneNode* ogreNode1 = worldNode->createChildSceneNode("ogreNode1");
    Entity* ogreEntity1 = scnMgr->createEntity("ogrehead.mesh");
    ogreNode1->attachObject(ogreEntity1);
    ogreNode1->setPosition(0, 0, 0);
    
    SceneNode* ogreNode2 = worldNode->createChildSceneNode("ogreNode2");
    Entity* ogreEntity2 = scnMgr->createEntity("ogrehead.mesh");
    ogreNode2->attachObject(ogreEntity2);
    ogreNode2->setPosition(0, 0, -100);
    
    //! [{gui settings}]
    scnMgr->addRenderQueueListener(getOverlaySystem());
    
    //! [TrayManager]
    mTrayMgr = new TrayManager("InterfaceName", getRenderWindow(), this);
    addInputListener(mTrayMgr);
    
    mTrayMgr->hideCursor();
    
    // Widgets
    mTrayMgr->createButton(TL_TOPLEFT, "MorningSkyBox", "MorningSkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "EarlyMorningSkyBox", "EarlyMorningSkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "StormySkyBox", "StormySkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "CloudyNoonSkyBox", "CloudyNoonSkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "SceneSkyBox2", "SceneSkyBox2");
    mTrayMgr->createButton(TL_TOPLEFT, "SpaceSkyBox", "SpaceSkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "TrippySkyBox", "TrippySkyBox");
    mTrayMgr->createButton(TL_TOPLEFT, "EveningSkyBox", "EveningSkyBox");
    mTrayMgr->createButton(TL_BOTTOMRIGHT, "releaseNode", "Release Node");
}

// Find all intersected nodes
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

// Handle click events (Save click position)
bool RollerCoaster::mousePressed(const MouseButtonEvent &evt)
{
    Camera* myCam {scnMgr->getCamera("myCam")};
    
    Ray mouseRay {
        myCam->getCameraToViewportRay(
            evt.x / float(myCam->getViewport()->getActualWidth()),
            evt.y / float(myCam->getViewport()->getActualHeight())
            )
        };

    std::vector<std::pair<SceneNode*, Vector3>> result {get_intersections(scnMgr->getSceneNode("worldNode"), mouseRay)};
    // node -> node2, node3
    // root -> node, camara
    if (not result.empty()) // Have colisions
    {
        // Save nereast intersected node and point intersection
        intersectedNode.first = result[0].first;
        intersectedNode.second = result[0].second;
        
        setHighlightedNode(result[0].first);
        
        mMovableFound = true; // Flag to mark a selection
        
        if (mTrayMgr->getWidget("xAxis") == nullptr)
        {
            // Show buttons to select axis to move nodes
            mTrayMgr->createButton(TL_BOTTOMLEFT, "xAxis", "X Axis");
            mTrayMgr->createButton(TL_BOTTOMLEFT, "yAxis", "Y Axis");
            mTrayMgr->createButton(TL_BOTTOMLEFT, "zAxis", "Z Axis");        
        }
    }
    else // Click over world
    {
        worldWasClicked = true;
    }   
    
    return true;
}

void RollerCoaster::resetHighlightedNode(void) noexcept
{
    if (highlightedNode != nullptr)
    {    
        highlightedNode->showBoundingBox(false);
        highlightedNode = nullptr;
    }
}

void RollerCoaster::setHighlightedNode(SceneNode* node) noexcept
{
    if (highlightedNode == nullptr)
    {
        highlightedNode = node;
        highlightedNode->showBoundingBox(true);
    }
    else
    {
        if (highlightedNode != node)
        {
            highlightedNode->showBoundingBox(false);
            
            highlightedNode = node;
            highlightedNode->showBoundingBox(true);
        }
    }
}

// Handle realese events (Save realese position)
bool RollerCoaster::mouseReleased(const MouseButtonEvent &evt)
{   
    worldWasClicked = false;
    return true;
}	

// Handle mouse movement events (Rotation direction, or move a node)
bool RollerCoaster::mouseMoved(const MouseMotionEvent &evt)
{    
    // evt: type, windowID, x, y, xrel, yrel
    Camera* myCam {scnMgr->getCamera("myCam")};
    
    Ray mouseRay {
        myCam->getCameraToViewportRay(
            evt.x / float(myCam->getViewport()->getActualWidth()),
            evt.y / float(myCam->getViewport()->getActualHeight())
            )
        };

    
    if (worldWasClicked) // Rotate scene
    {    
        rotateScene(evt.xrel, evt.yrel); // rotate based on relative mouse speed
    }
    
    return true;
}

// Handle mouse wheel events (Zoom in or Zoom out)
bool RollerCoaster::mouseWheelRolled(const MouseWheelEvent &evt) // Zoom in/out
{
    auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
    auto cameraNode {scnMgr->getSceneNode("camNode")};
    cameraNode->translate(evt.y * direction * 1.5);

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
        cameraNode->pitch(Degree(5));
    }
    else if (evt.keysym.sym == SDLK_DOWN) // Down arrow : rotate x- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->pitch(Degree(-5));
    }
    else if (evt.keysym.sym == SDLK_LEFT) // Left arrow : rotate y- camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->yaw(Degree(5));
    }
    else if (evt.keysym.sym == SDLK_RIGHT) // Right arrow : rotate y+ camera
    {
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->yaw(Degree(-5));
    }
    else if (evt.keysym.sym == 119) // Key "w" : move up camera
    {
        auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
        if (highlightedNode != nullptr)
        {
            highlightedNode->translate(direction * 3.5);
        }
        auto cameraNode {scnMgr->getSceneNode("camNode")};
        cameraNode->translate(direction * 3.5);
    }
    else if (evt.keysym.sym == 115) //Key "s" : move down camera
    {
        auto direction {scnMgr->getCamera("myCam")->getRealDirection()};
        if (highlightedNode != nullptr)
        {
            highlightedNode->translate(-direction * 3.5);
        }
        auto cameraNode {scnMgr->getSceneNode("camNode")};
        cameraNode->translate(-direction * 3.5);
    }
    
    else if (evt.keysym.sym == 97) // Key "a" : move left camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        if (highlightedNode != nullptr)
        {
            highlightedNode->translate(-direction * 3.5);
        }
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(-direction * 3.5);
    }
    else if (evt.keysym.sym == 100) // Key "d" : move right camera
    {
        auto direction = scnMgr->getCamera("myCam")->getRealRight();
        if (highlightedNode != nullptr)
        {
            highlightedNode->translate(direction * 3.5);
        }
        auto cameraNode = scnMgr->getSceneNode("camNode");
        cameraNode->translate(direction * 3.5);
    }
    
    return true;
}

// Aply a rotation based on mouse speed
void RollerCoaster::rotateScene(int speedX, int speedY) noexcept
{
    // Moving camNode to TempNode
    SceneNode* camNode {scnMgr->getSceneNode("camNode")};
    SceneNode* fatherCamera {camNode->getParentSceneNode()};

    SceneNode* tempNode;
    if (not scnMgr->hasSceneNode("tempNode"))
    {    
        tempNode = fatherCamera->createChildSceneNode("tempNode", camNode->getPosition(), camNode->getOrientation());
    }
    else
    {    
        tempNode = scnMgr->getSceneNode("tempNode");
    }
    
    
    Vector3 resultPosition = tempNode->convertWorldToLocalPosition(camNode->getPosition());
    Quaternion resultOrientation = tempNode->convertWorldToLocalOrientation(camNode->getOrientation());

    fatherCamera->removeChild(camNode);
    tempNode->addChild(camNode);
    camNode->setPosition(resultPosition);
    camNode->setOrientation(resultOrientation);
    camNode->setInheritScale(false);
    
    // Moving highlightedNode to TempNode
    SceneNode* fatherhighlightedNode {highlightedNode->getParentSceneNode()};
    
    Vector3 resultPosition2 = tempNode->convertWorldToLocalPosition(highlightedNode->getPosition());
    Quaternion resultOrientation2 = tempNode->convertWorldToLocalOrientation(highlightedNode->getOrientation());

    fatherhighlightedNode->removeChild(highlightedNode);
    tempNode->addChild(highlightedNode);
    highlightedNode->setPosition(resultPosition2);
    highlightedNode->setOrientation(resultOrientation2);
    highlightedNode->setInheritScale(false);
    
    
    if (highlightedNode != nullptr)
    {        
        tempNode->yaw(Degree(speedX * 0.0001));
        tempNode->pitch(Degree(speedY * 0.0001));
    }
    
    // Back camNode
    resultPosition = fatherCamera->convertWorldToLocalPosition(camNode->getPosition());
    resultOrientation = fatherCamera->convertWorldToLocalOrientation(camNode->getOrientation());

    tempNode->removeChild(camNode);
    fatherCamera->addChild(camNode);
    camNode->setPosition(resultPosition);
    camNode->setOrientation(resultOrientation);
    camNode->setInheritScale(false);
    
    // Back highlightedNode
    resultPosition2 = fatherhighlightedNode->convertWorldToLocalPosition(highlightedNode->getPosition());
    resultOrientation2 = fatherhighlightedNode->convertWorldToLocalOrientation(highlightedNode->getOrientation());

    tempNode->removeChild(highlightedNode);
    fatherhighlightedNode->addChild(highlightedNode);
    highlightedNode->setPosition(resultPosition2);
    highlightedNode->setOrientation(resultOrientation2);
    highlightedNode->setInheritScale(false);
}

// Override from TrayListener to manage click events in buttons
void RollerCoaster::buttonHit(Button * button)
{
    if (button->getName().compare("releaseNode") == 0)
    {
        resetHighlightedNode();
        if (mMovableFound)
        {
            mMovableFound = false; // Release selected node
        }
    }
    else
    {
        std::string materialName {"Examples/" + button->getCaption()};
        scnMgr->setSkyBox(true,materialName);
    }
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
