#ifndef ENGINE_FOLLOWPATH
#define ENGINE_FOLLOWPATH

#include "Engine.hpp"
#include "UpdateNode3D.hpp"
#include "Relations.hpp"

class FollowPath : public UpdateNode3D
{
    /// Points that form the path of the object
    std::vector<glm::vec3> pathPoints;
    /// The node that will follow the path
    Node3D* walker;
    /// How fast the node moves along the path
    float speed;
    /// If true, will move towards the first point in the path when reaching the end instead of going backwards
    bool circular;
    /// If true, will rotate the target to that it faces the target point
    bool doRotation;

    int direction = 1;
    int targetIndex = 0;

    void nextTarget()
    {
        log(std::format("Path walker {} has reached point {}-{}", walker, targetIndex, pathPoints[targetIndex]));
        if (circular)
        {
            targetIndex++;
            targetIndex = targetIndex % pathPoints.size();
        }
        else
        {
            if (targetIndex + direction == pathPoints.size() || targetIndex + direction == -1)
                direction *= -1;
            targetIndex += direction;
        }
    }

public:
    FollowPath(const std::vector<glm::vec3>& path, Node3D* target, const float speed = 1, const bool circular = false, const bool doRotation = false)
    {
        this->pathPoints = path;
        this->walker = target;
        this->speed = speed;
        this->circular = circular;
        this->doRotation = doRotation;
    }

    void update() override
    {
        //log(std::format("target @ {} to {} {}", target->getGlobalPosition(), pathPoints[targetIndex], targetIndex));
        if (glm::all(glm::epsilonEqual(walker->getGlobalPosition(), pathPoints[targetIndex], EPSILON * 100)))
        {
            walker->setGlobalPosition(pathPoints[targetIndex]);
            nextTarget();
        }

        const glm::vec3 dir = glm::normalize(pathPoints[targetIndex] - walker->getGlobalPosition());
        const glm::vec3 delta = dir * (speed * Engine::getDeltaTime());
        //log(std::format("dir: {} {}", dir, delta));
        walker->globalTranslate(delta);

        //if (doRotation) TODO fix look at
        //    target->lookAt(pathPoints[targetIndex]);
    }
};
#endif