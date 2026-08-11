/// 2DNode
#ifndef ENGINE_NODE2D_H
#define ENGINE_NODE2D_H

#include "Node.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include <cmath>

#define MAT4_I glm::mat4(1.0f)
#define VEC2_X glm::vec2(1.0f, 0.0f)
#define VEC2_Y glm::vec2(0.0f, 1.0f)
#define VEC2_ZERO glm::vec2(0.0f)
#define VEC2_ONE glm::vec2(1.0f)
#define PLANE_ROTATION glm::vec3(0.0f, 0.0f, 1.0f)
#define X_ROTATION_INDEX 0
#define Y_ROTATION_INDEX 1
#define POSITION_INDEX 3

/**
 *  Node that has a place in the 2D plane
 *
 *  The logic implementation is similar to the one of the Node3D, but with one less dimension
 *
 * */
class Node2D: public Node {

    public:

        glm::mat4 matrix;
        glm::vec2 position;
        float rotation;
        glm::vec2 scale;

        glm::mat4 localMatrix;
        glm::vec2 localPosition;
        float localRotation;
        glm::vec2 localScale;

        Node2D() {
            this->position = this->localPosition = VEC2_ZERO;
            this->rotation = this->localRotation = 0.0f;
            this->scale = this->localScale = VEC2_ONE;
            this->matrix = this->localMatrix = MAT4_I;
        }

        Node2D(const glm::vec2 position, const float rotation, const glm::vec2 scale) {
            this->position = this->localPosition = position;
            this->rotation = this->localRotation = rotation;
            this->scale = this->localScale = scale;
            this->matrix = this->localMatrix = this->computeLocalMatrixFromTransform(position, rotation, scale);
        }

        glm::mat4 computeLocalMatrixFromTransform(glm::vec2 position, float rotation, glm::vec2 scale) {
            return
                glm::translate(MAT4_I, glm::vec3(position, 0)) *
                glm::rotate(MAT4_I, rotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 0)) *
                MAT4_I;
        }

        /**
         *  Moves the node of the given distance
         * */
        void translate(const glm::vec2 distance) {
            this->localMatrix = glm::translate(MAT4_I, glm::vec3(distance, 0)) * this->localMatrix;
            this->localPosition += distance;

            this->commitUpdate();
        }

        /**
         *  Rotates the node of the given angle
         * */
        void rotate(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0)) *
                glm::rotate(MAT4_I, angle, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0)) *
                this->localMatrix;
            this->localRotation += angle;

            this->commitUpdate();
        }

        /**
         *  Scales the node of the given scale
         * */
        void scaleAll(const glm::vec2 scale) {
            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0)) *
                glm::rotate(MAT4_I, this->localRotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 0)) *
                glm::rotate(MAT4_I, -this->localRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0)) *
                this->localMatrix;
            this->localScale *= scale;

            this->commitUpdate();
        }

        /**
         *  Returns the direction of the local X axis
         * */
        glm::vec2 getLocalXAxis() const {
            return glm::normalize(glm::vec2(this->matrix[X_ROTATION_INDEX]));
        }

        /**
         *  Returns the direction of the local Y axis
         * */
        glm::vec2 getLocalYAxis() const {
            return glm::normalize(glm::vec2(this->matrix[Y_ROTATION_INDEX]));
        }

        /** Commits an update from the node */
        void commitUpdate() {

            // Commit update to self and to the node's children
            this->updateGlobalTransform(this, MAT4_I);
        }

        /** Recursively updates the node and its children and so on */
        void updateGlobalTransform(Node *node, glm::mat4 fatherTransformMatrix) {

            // Update self
            // If node id Node2D, update it, else skip to its children
            if (Node2D* node2d = dynamic_cast<Node2D*>(node)) {
                node2d->updateGlobalMatrix(fatherTransformMatrix);
                node2d->updateTransformProperties();
                fatherTransformMatrix = node2d->matrix;
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
         *  Updates the global transform from the global matrix
         * */
        void updateTransformProperties() {
            this->position = glm::vec2(this->matrix[POSITION_INDEX]);

            // Compute the angle
            const glm::vec2 xAxis = this->getLocalXAxis();
            this->rotation = atan2(xAxis[0], xAxis[1]);

            // Use geometry to compute the scale
            this->scale = glm::vec2(
                    glm::length(glm::vec2(this->matrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec2(this->matrix[Y_ROTATION_INDEX]))
                    );
        }

        static Node2D* fromJSON(const nlohmann::json& json) {
            Node2D *newNode = new Node2D();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec2 position = VEC2_ZERO;
            float rotation = 0.0f;
            glm::vec2 scale = VEC2_ONE;
            if (json.contains("position")) position = glm::vec2(json["position"][0].get<float>(), json["position"][1].get<float>());
            if (json.contains("rotation")) rotation = json["rotation"].get<float>();
            if (json.contains("scale")) scale = glm::vec2(json["scale"][0].get<float>(), json["scale"][1].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            return newNode;
        }
};

#endif
