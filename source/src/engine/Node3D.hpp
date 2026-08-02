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

/// A Node that has a place in 3D space
class Node3D : public Node {

    public:

        /// Node's transformation matrix
        glm::mat4 matrix;

        /// Node's local transformation matrix (if no 3D predecessor, same as global)
        glm::mat4 localMatrix;

        /* Default constructor */
        Node3D() : Node() {
            this->matrix = MAT4_I;
            this->localMatrix = MAT4_I;
        }

        /* Constructor with parameters */
        Node3D(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node() {
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

        /* Computes the local transform matrix from the given one */
        glm::mat4 computeLocalTransformMatrix(glm::mat4 startingMatrix) {
            return glm::inverse(startingMatrix) * this->matrix;
        }

        /* Computes the local coordinates of the given point from the node's position */
        glm::vec3 toLocalSpace(glm::vec3 point) {
            glm::mat4 newLocalMatrix = glm::inverse(this->matrix) * glm::translate(MAT4_I, point);
            return glm::vec3(newLocalMatrix[3][0], newLocalMatrix[3][1], newLocalMatrix[3][2]);
        }

        /* gets the nodes's local X axis */
        glm::vec3 getLocalXAxis() const {
            return glm::normalize(glm::vec3(this->matrix[0]));
        }

        /* gets the nodes's local Y axis */
        glm::vec3 getLocalYAxis() const {
            return glm::normalize(glm::vec3(this->matrix[1]));
        }

        /* gets the nodes's local Z axis */
        glm::vec3 getLocalZAxis() const {
            return glm::normalize(glm::vec3(this->matrix[2]));
        }

        /* Rotate the node around the x axis parallel */
        void rotateX(float angle) {
            glm::vec3 position = this->getPosition();
            glm::vec3 rotation = this->getRotation();
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

        /* Rotate the node around the y axis parallel */
        void rotateY(float angle) {
            glm::vec3 position = this->getPosition();
            glm::vec3 rotation = this->getRotation();
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

        /* Rotate the node around the z axis parallel */
        void rotateZ(float angle) {
            glm::vec3 position = this->getPosition();
            glm::vec3 rotation = this->getRotation();
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

        /* Sets a new position to the node */
        void translate(glm::vec3 newPosition) {
            this->matrix = glm::translate(MAT4_I, newPosition - this->getPosition()) * this->matrix;
        }

        /* Returns the position of the node */
        glm::vec3 getPosition() {
            return glm::vec3(this->matrix[3][0], this->matrix[3][1], this->matrix[3][2]);
        }

        /* Returns a vector with the rotation of the ndoe (in rad) */
        glm::vec3 getRotation() {
            glm::vec3 xAxis = this->getLocalXAxis();
            glm::vec3 yAxis = this->getLocalYAxis();
            glm::vec3 zAxis = this->getLocalZAxis();
            // Compute rotation from the rotationb matrix (the local axises)
            return glm::vec3(
                    -std::atan2(zAxis[1], zAxis[2]),
                    std::asin(zAxis[0]),
                    std::atan2(yAxis[0], xAxis[0]));
        }

        // TODO: scale
};

#endif
