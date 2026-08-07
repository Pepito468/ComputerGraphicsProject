// 3DNode
#ifndef ENGINE_NODE3D_H
#define ENGINE_NODE3D_H

#include "Node.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include <format>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        // Node's local matrix
        glm::mat4 localMatrix;
        glm::vec3 localPosition;
        glm::vec3 localRotation;
        glm::vec3 localScale;

        /// Default constructor
        Node3D() : Node() {
            this->position = this->localPosition = VEC3_ZERO;
            this->rotation = this->localRotation = VEC3_ZERO;
            this->scale = this->localScale = VEC3_ONE;
            this->matrix = this->localMatrix = MAT4_I;
        }

        /// Constructor with parameters
        Node3D(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node() {
            if (scale.x == 0 || scale.y == 0 || scale.z == 0) {
                // A scale of 0 produces nan after a rotation
                error(std::format("Trying to set scale for [{}] to 0", this->name));
            }

            this->position = this->localPosition = position;
            this->rotation = this->localRotation = rotation;
            this->scale = this->localScale = scale;
            this->matrix = this->localMatrix =
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                MAT4_I;
        }

        /// Rotates the node around the X-axis parallel
        void rotateX(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, angle, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.x += angle;

            this->commitUpdate();
        }

        /// Rotate the node around the Y-axis parallel
        void rotateY(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, angle, VEC3_Y) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.y += angle;

            this->commitUpdate();
        }

        /// Rotate the node around the Z-axis parallel
        void rotateZ(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, angle, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.z += angle;

            this->commitUpdate();
        }

        /// Translates the node
        void translate(const glm::vec3 distance) {
            this->localMatrix = glm::translate(MAT4_I, distance) * this->localMatrix;
            this->localPosition += distance;

            this->commitUpdate();
        }

        /**
         * Scales the node by the given amount
         * */
        void scaleAll(const glm::vec3 scale) {
            // Scale cannot be 0
            if (scale.x == 0 || scale.y == 0 or scale.z == 0)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->name));

            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->localRotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localScale *= scale;

            this->commitUpdate();
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

        /** Commits an update from the node */
        void commitUpdate() {

            // Commit update to self and to the node's children
            this->updateGlobalTransform(this, MAT4_I);
        }

        /** Recursively updates the node and its children and so on */
        void updateGlobalTransform(Node *node, glm::mat4 fatherTransformMatrix) {

            // Update self
            // If node id Node3D, update it, else skip to its children
            if (Node3D* node3d = dynamic_cast<Node3D*>(node)) {
                node3d->updateGlobalMatrix(fatherTransformMatrix);
                node3d->updateTransformProperties();
                fatherTransformMatrix = node3d->matrix;
            }

            // Propagate to children
            for (Node *child : node->children) {
                updateGlobalTransform(child, fatherTransformMatrix);
            }

        }

        /**
         *  Updates the global matrix from the local and the given the father's matrix
         * */
        void updateGlobalMatrix(glm::mat4 fatherMatrix) {
            this->matrix = fatherMatrix * this->localMatrix;
        }

        /**
         *  Updates the global transform properties of the node from the matrix (position, rotation, scale).
         *  *MUST* be called after updating the global matrix to avoid data corruption
         * */
        void updateTransformProperties() {

            // Position
            this->position = glm::vec3(this->matrix[POSITION_INDEX]);

            // Rotation
            // Compute rotation from the rotation matrix (the local axises) using geometry
            const glm::vec3 xAxis = this->getLocalXAxis();
            const glm::vec3 yAxis = this->getLocalYAxis();
            const glm::vec3 zAxis = this->getLocalZAxis();
            this->rotation =  glm::vec3(
                    -std::atan2(zAxis[1], zAxis[2]),
                    std::asin(zAxis[0]),
                    -std::atan2(yAxis[0], xAxis[0])
                    );

            // Scale
            // Use Pythagoras' theorem to get the length of the axis
            this->scale = glm::vec3(
                    glm::length(glm::vec3(this->matrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->matrix[Y_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->matrix[Z_ROTATION_INDEX]))
                    );

        }

        /// Computes the local coordinates of the given point from the node's position
        glm::vec3 toLocalSpace(const glm::vec3 point) const {
            const glm::mat4 newLocalMatrix = glm::inverse(this->localMatrix) * glm::translate(MAT4_I, point);
            return glm::vec3(newLocalMatrix[POSITION_INDEX]);
        }

        /// Computes the global coordinates of the given point
        glm::vec3 toGlobalSpace(const glm::vec3 point) const
        {
            const glm::mat4 newGlobalMatrix = this->matrix * glm::translate(MAT4_I, point);
            return glm::vec3(newGlobalMatrix[POSITION_INDEX]);
        }

};

#endif
