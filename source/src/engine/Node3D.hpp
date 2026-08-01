// 3DNode
#ifndef ENGINE_NODE3D_H
#define ENGINE_NODE3D_H
#define GLM_FORCE_RADIANS

#include "Node.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// Keeps track of a node's spacial characteristics
struct Transform
{
    /// The transform's position
    glm::vec3 position;
    /// The transform's rotation, in radians
    glm::vec3 rotation;
    /// The transform's scale
    glm::vec3 scale;

    Transform() : position(glm::vec3(0)), rotation(glm::vec3(0)), scale(glm::vec3(1)) {}
    Transform(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : position(position), rotation(rotation), scale(scale) {}

    glm::mat4 calculateTransformationMatrix() const
    {
        const glm::mat4 transformMat =
            glm::translate(glm::mat4(1.0), position) *
            glm::rotate(glm::mat4(1.0), rotation.y , glm::vec3(0,1,0)) *
            glm::rotate(glm::mat4(1.0), rotation.x , glm::vec3(1,0,0)) *
            glm::rotate(glm::mat4(1.0), rotation.z , glm::vec3(0,0,1)) *
            glm::scale(glm::mat4(1.0), scale);
        return transformMat;
    }

    glm::vec3 getXAxis() const
    {
        return
        glm::rotate(glm::mat4(1.0), rotation.y , glm::vec3(0,1,0)) *
            glm::rotate(glm::mat4(1.0), rotation.x , glm::vec3(1,0,0)) *
            glm::rotate(glm::mat4(1.0), rotation.z , glm::vec3(0,0,1)) *
                glm::vec4(1, 0, 0, 1);
    }

    glm::vec3 getYAxis() const
    {
        return
        glm::rotate(glm::mat4(1.0), rotation.y , glm::vec3(0,1,0)) *
            glm::rotate(glm::mat4(1.0), rotation.x , glm::vec3(1,0,0)) *
            glm::rotate(glm::mat4(1.0), rotation.z , glm::vec3(0,0,1)) *
                glm::vec4(0, 1, 0, 1);
    }

    glm::vec3 getZAxis() const
    {
        return
        glm::rotate(glm::mat4(1.0), rotation.y , glm::vec3(0,1,0)) *
            glm::rotate(glm::mat4(1.0), rotation.x , glm::vec3(1,0,0)) *
            glm::rotate(glm::mat4(1.0), rotation.z , glm::vec3(0,0,1)) *
                glm::vec4(0, 0, 1, 1);
    }

    /**
     * Calculates a new transform that represents this one in another transform's local space.
     * @param newSpace The transform to use as a reference system.
     * @return A new transform instance.
     */
    Transform localized(const Transform newSpace) const
    {
        return Transform(
            position - newSpace.position,
            rotation - newSpace.rotation, //TODO
            scale / newSpace.scale
        );
    }

    /**
     * Calculates a new transform that represents this one in global space.
     * @param oldSpace The transform to use as a reference system.
     * @return A new transform instance.
     */
    Transform globalized(const Transform oldSpace) const
    {
        return Transform(
            position + oldSpace.position,
            rotation + oldSpace.rotation,//TODO
            scale + oldSpace.scale
        );
    }

    /**
     * Calculate's the point's coordinates in the transform's local space.
     * @param point The point to use.
     * @return A new vec3 instance.
     */
    glm::vec3 toLocalSpace(const glm::vec3 point) const
    {
        return calculateTransformationMatrix() * glm::vec4(point, 1);
    }
};

/// A Node that has a place in 3D space
class Node3D : public Node {
    public:
        /// The node's transform in world space
        Transform globalTransform;

        /// The node's transform relative to its parent (if no 3D parent, same as global)
        Transform localTransform;

        /**
         * Adds a node to this node's children and updates its localTransform. \n
         * @copydoc Node::adopt
         */
        void adopt(Node3D* child)
        {
            Node::adopt(child);
            child->localTransform = child->globalTransform.localized(globalTransform);
        }

        /**
         * Removes a node from this node's children and updates its localTransform. \n
         * @copydoc Node::adopt
         */
        void disown(Node3D* child)
        {
            Node::disown(child);
            child->localTransform = child->globalTransform;
        }
};

#endif
