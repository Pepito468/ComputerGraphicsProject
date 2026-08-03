// 3DNode
#ifndef ENGINE_NODE3D_H
#define ENGINE_NODE3D_H

#include "Node.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAT4_I glm::mat4(1.0f)
#define VEC3_X glm::vec3(1.0f, 0.0f, 0.0f)
#define VEC3_Y glm::vec3(0.0f, 1.0f, 0.0f)
#define VEC3_Z glm::vec3(0.0f, 0.0f, 1.0f)
#define VEC3_ZERO glm::vec3(0.0f)
#define VEC3_ONE glm::vec3(1.0f)
#define X_ROTATION_INDEX 0
#define Y_ROTATION_INDEX 1
#define Z_ROTATION_INDEX 2
#define POSITION_INDEX 3

/**
 * A Node that has a place in 3D space
 *
 * NOTES:
 *      the rotation convention is ZYX
 *      the rotation angles are in radians
 *
 */
class Node3D : public Node {

    public:

        /// Node's transformation matrix
        glm::mat4 matrix;

        /// Node's local transformation matrix (if no 3D predecessor, same as global)
        glm::mat4 localMatrix;

        /// Default constructor
        Node3D() : Node() {
            this->matrix = MAT4_I;
            this->localMatrix = MAT4_I;
        }

        /// Constructor with parameters
        Node3D(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node() {
            if (scale.x == 0 || scale.y == 0 || scale.z == 0) {
                // A scale of 0 produces nan after a rotation
                error("Flattening, at least one of the scaling factors is 0");
            }
            
            this->matrix = 
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) * 
                MAT4_I;
            this->localMatrix = this->matrix;
        }

        /**
         * Adds a node to this node's children and updates its localTransform. \n
         * @copydoc Node::adopt
         */
        void adopt(Node3D* child) {
            Node::adopt(child);
            child->localMatrix = child->computeLocalTransformMatrix(this->matrix);
        }

        /**
         * Removes a node from this node's children and updates its localTransform. \n
         * @copydoc Node::adopt
         */
        void disown(Node3D* child) {
            Node::disown(child);
            child->localMatrix = child->matrix;
        }

        /// Computes the local transform matrix from the given one
        glm::mat4 computeLocalTransformMatrix(const glm::mat4& startingMatrix) const {
            return glm::inverse(startingMatrix) * this->matrix;
        }

        /// Computes the local coordinates of the given point from the node's position
        glm::vec3 toLocalSpace(const glm::vec3 point) const {
            glm::mat4 newLocalMatrix = glm::inverse(this->matrix) * glm::translate(MAT4_I, point);
            return glm::vec3(newLocalMatrix[POSITION_INDEX]);
        }

        /// Gets the node's local X-axis
        glm::vec3 getLocalXAxis() const {
            return glm::normalize(glm::vec3(this->matrix[X_ROTATION_INDEX]));
        }

        /// Gets the node's local Y-axis
        glm::vec3 getLocalYAxis() const {
            return glm::normalize(glm::vec3(this->matrix[Y_ROTATION_INDEX]));
        }

        /// Gets the node's local Z-axis
        glm::vec3 getLocalZAxis() const {
            return glm::normalize(glm::vec3(this->matrix[Z_ROTATION_INDEX]));
        }

        /// Rotate the node around the X-axis parallel
        void rotateX(const float angle) {
            const glm::vec3 position = this->getPosition();
            const glm::vec3 rotation = this->getRotation();
            this->matrix =
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, angle, VEC3_X) *
                glm::rotate(MAT4_I, -rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -position) *
                this->matrix;
        }

        /// Rotate the node around the Y-axis parallel
        void rotateY(const float angle) {
            const glm::vec3 position = this->getPosition();
            const glm::vec3 rotation = this->getRotation();
            this->matrix =
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, angle, VEC3_Y) *
                glm::rotate(MAT4_I, -rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.x, VEC3_X) *
                glm::translate(MAT4_I, -position) *
                this->matrix;
        }

        /// Rotate the node around the Z-axis parallel
        void rotateZ(const float angle) {
            const glm::vec3 position = this->getPosition();
            const glm::vec3 rotation = this->getRotation();
            this->matrix =
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, angle, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, -rotation.x, VEC3_X) *
                glm::translate(MAT4_I, -position) *
                this->matrix;
        }

        /// Sets the angle for the node from the given angles (in rad)
        void setRotation(const float alpha, const float beta, const float gamma) {
            const glm::vec3 position = this->getPosition();
            const glm::vec3 rotation = this->getRotation();
            this->matrix = 
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, alpha, VEC3_X) *
                glm::rotate(MAT4_I, beta, VEC3_Y) *
                glm::rotate(MAT4_I, gamma, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, -rotation.x, VEC3_X) *
                glm::translate(MAT4_I, -position) *
                this->matrix;
        }

        /// Sets a new position to the node
        void translate(const glm::vec3 newPosition) {
            this->matrix = glm::translate(MAT4_I, newPosition - this->getPosition()) * this->matrix;
        }

        /// Sets the node's scale
        void setScale(const glm::vec3 newScale) {
            // Scale cannot be 0
            if (newScale.x == 0 || newScale.y == 0 or newScale.z == 0)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->name));

            const glm::vec3 position = this->getPosition();
            const glm::vec3 rotation = this->getRotation();
            const glm::vec3 scale = this->getScale();
            this->matrix =
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::scale(MAT4_I, newScale / scale) *
                glm::rotate(MAT4_I, -rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, -rotation.x, VEC3_X) *
                glm::translate(MAT4_I, -position) *
                this->matrix;
        }

        /// Returns the position of the node
        glm::vec3 getPosition() const {
            return glm::vec3(this->matrix[POSITION_INDEX]);
        }

        /// Returns a vector with the rotation of the node (in rad)
        glm::vec3 getRotation() const {
            glm::vec3 xAxis = this->getLocalXAxis();
            glm::vec3 yAxis = this->getLocalYAxis();
            glm::vec3 zAxis = this->getLocalZAxis();
            // Compute rotation from the rotation matrix (the local axes) using geometry
            return glm::vec3(
                    -std::atan2(zAxis[1], zAxis[2]),
                    std::asin(zAxis[0]),
                    -std::atan2(yAxis[0], xAxis[0]));
        }

        /// Returns the vector containing the scale of the node
        glm::vec3 getScale() const {
            // Use Pythagoras' theorem to get the length of the axis
            return glm::vec3(
                    glm::length(glm::vec3(this->matrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->matrix[Y_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->matrix[Z_ROTATION_INDEX]))
                    );
        }
};

#endif
