#include "Physics.hpp"
#include "Debug.hpp"
#include "Relations.hpp"
#include <format>
#include <memory>

//Flags
std::string w1ColID = "", w2ColID = "", w2TrigEntID = "", w2TrigStayID = "", w2TrigExtID = "";
bool w1Trig = false, boxColl = false;

void w1OnColl(Collider* other) {w1ColID = other->name;}
void w1OnTrig(Collider* other) {w1Trig = true;}
void w2OnColl(Collider* other) {w2ColID = other->name;}
void w2OnTrigEnt(Collider* other) {w2TrigEntID = other->name;}
void w2OnTrigStay(Collider* other) {w2TrigStayID = other->name;}
void w2OnTrigExt(Collider* other) {w2TrigExtID = other->name;}
void boxOnColl(Collider* other) {boxColl = true;}

int main()
{
    println("PHYSICS TEST");

    //Set up the scene
    Node3D root = Node3D();
    root.name = "root";
    BoxCollider box = BoxCollider();
    box.name = "box";
    BoxCollider wall1 = BoxCollider();
    wall1.name = "wall1";
    BoxCollider wall2 = BoxCollider();
    wall2.name = "wall2";

    root.adopt(&box);
    root.adopt(&wall1);
    root.adopt(&wall2);

    wall1.globalTranslate({5, 0, 0});
    wall1.depth = 5;
    wall1.movementStatus = STATIC;

    wall1.onCollision = w1OnColl;
    wall1.onTriggerEnter = w1OnTrig;
    wall1.onTriggerExit = w1OnTrig;
    wall1.onTriggerStay = w1OnTrig;

    wall2.globalTranslate({0, 0, 5});
    wall2.globalScaleAll({5, 1, 1});
    wall2.movementStatus = STATIC;
    wall2.isTrigger = true;

    wall2.onCollision = w2OnColl;
    wall2.onTriggerEnter = w2OnTrigEnt;
    wall2.onTriggerExit = w2OnTrigExt;
    wall2.onTriggerStay = w2OnTrigStay;

    box.onCollision = boxOnColl;
    box.onTriggerEnter = boxOnColl;
    box.onTriggerExit = boxOnColl;
    box.onTriggerStay = boxOnColl;

    println("\tLOADING SCENE");
    Physics::loadScene(&root);

    println("\tNO MOVEMENT");
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID.empty(), std::format("Wall1 collided with {}", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID.empty(), std::format("Wall2 trigger-enter with {}", w2TrigEntID));
    _assert(w2TrigExtID.empty(), std::format("Wall2 trigger-exit with {}", w2TrigExtID));
    _assert(w2TrigStayID.empty(), std::format("Wall2 trigger-stay with {}", w2TrigStayID));

    println("\tSMALL MOVEMENT");
    box.globalTranslate({2, 0, 0});
    _assert(box.movementStatus == MOBILE_HAS_MOVED, "Box marked as unmoved");
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID.empty(), std::format("Wall1 collided with {}", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID.empty(), std::format("Wall2 trigger-enter with {}", w2TrigEntID));
    _assert(w2TrigExtID.empty(), std::format("Wall2 trigger-exit with {}", w2TrigExtID));
    _assert(w2TrigStayID.empty(), std::format("Wall2 trigger-stay with {}", w2TrigStayID));

    println("\tCOLLIDE WITH WALL 1");
    box.globalTranslate({3, 0, 0});
    _assert(box.movementStatus == MOBILE_HAS_MOVED, "Box marked as unmoved");
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(box.getGlobalPosition().x == 2, "Box movement not reversed");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID == box.name, std::format("Wall1 collided with {} instead ox box", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID.empty(), std::format("Wall2 trigger-enter with {}", w2TrigEntID));
    _assert(w2TrigExtID.empty(), std::format("Wall2 trigger-exit with {}", w2TrigExtID));
    _assert(w2TrigStayID.empty(), std::format("Wall2 trigger-stay with {}", w2TrigStayID));
    w1ColID = "";

    println("\tTRIGGER ENTER WALL 2");
    box.globalTranslate({-2, 0, 5});
    _assert(box.movementStatus == MOBILE_HAS_MOVED, "Box marked as unmoved");
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID.empty(), std::format("Wall1 collided with {}", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID == box.name, std::format("Wall2 trigger-enter with {}, instead of box", w2TrigEntID));
    _assert(w2TrigExtID.empty(), std::format("Wall2 trigger-exit with {}", w2TrigExtID));
    _assert(w2TrigStayID.empty(), std::format("Wall2 trigger-stay with {}", w2TrigStayID));
    w2TrigEntID = "";

    println("\tTRIGGER STAY WALL 2");
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID.empty(), std::format("Wall1 collided with {}", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID.empty(), std::format("Wall2 trigger-enter with {}", w2TrigEntID));
    _assert(w2TrigExtID.empty(), std::format("Wall2 trigger-exit with {}", w2TrigExtID));
    _assert(w2TrigStayID == box.name, std::format("Wall2 trigger-stay with {} instead of box", w2TrigStayID));
    w2TrigStayID = "";

    println("\tTRIGGER EXIT WALL 2");
    box.globalTranslate({0, 0, -5});
    Physics::checkCollisions();
    _assert(box.movementStatus == MOBILE_UNMOVED, "Box marked as moved");
    _assert(!boxColl, "Box collided");
    _assert(w1ColID.empty(), std::format("Wall1 collided with {}", w1ColID));
    _assert(!w1Trig, "Wall1 triggered");
    _assert(w2ColID.empty(), std::format("Wall2 collided with {}", w2ColID));
    _assert(w2TrigEntID.empty(), std::format("Wall2 trigger-enter with {}", w2TrigEntID));
    _assert(w2TrigExtID == box.name, std::format("Wall2 trigger-exit with {} instead of box", w2TrigExtID));
    _assert(w2TrigStayID.empty(), std::format("Wall2 trigger-stay with {}", w2TrigStayID));

    Physics::RaycastHit hit = Physics::RaycastHit();
    bool contact = false;

    println("\tRAYCAST TEST -- No hit");
    contact = Physics::raycast({-5, -5, 0}, {-1, 3, -2}, &hit, 5);
    _assert(!contact, "A hit was registered");
    _assert(epsilonEqual(hit.point, VEC3_ZERO) && epsilonEqual(hit.distance, 0) && hit.collider == nullptr, std::format("Hit was filled out: {}, {}", hit.point, hit.distance, hit.collider ? hit.collider->name : ""));

    println("\tRAYCAST TEST -- Box hit");
    contact = Physics::raycast(VEC3_ZERO, {-1, 3, -2}, &hit);
    _assert(contact, "No hit was registered");
    _assert(epsilonEqual(hit.point, VEC3_ZERO) && epsilonEqual(hit.distance, 0) && hit.collider && hit.collider->UUID == box.UUID, std::format("Wrong hit: {}, {}, {}", hit.point, hit.distance, hit.collider ? hit.collider->name : ""));

    println("\tRAYCAST TEST -- Wall 1 hit");
    box.isActive = false;
    contact = Physics::raycast(VEC3_ZERO, VEC3_X, &hit);
    _assert(contact, "No hit was registered");
    _assert(epsilonEqual(hit.point, {4.5f, 0, 0}) && epsilonEqual(hit.distance, 4.5f) && hit.collider && hit.collider->UUID == wall1.UUID, std::format("Wrong hit: {}, {}, {}", hit.point, hit.distance, hit.collider ? hit.collider->name : ""));

    println("\tRAYCAST TEST -- Wall 2 hit");
    contact = Physics::raycast(VEC3_ZERO, VEC3_Z, &hit);
    _assert(contact, "No hit was registered");
    _assert(epsilonEqual(hit.point, {0, 0, 4.5f}) && epsilonEqual(hit.distance, 4.5f) && hit.collider && hit.collider->UUID == wall2.UUID, std::format("Wrong hit: {}, {}, {}", hit.point, hit.distance, hit.collider ? hit.collider->name : ""));

    println("END PHYSICS TEST");
}