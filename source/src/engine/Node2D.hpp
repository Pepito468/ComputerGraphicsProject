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

    private:

        glm::mat4 globalMatrix;
        glm::vec2 globalPosition;
        float globalRotation;
        glm::vec2 globalScale;

        glm::mat4 fatherMatrix;

        glm::mat4 localMatrix;
        glm::vec2 localPosition;
        float localRotation;
        glm::vec2 localScale;

    public:

        Node2D() {
            this->globalPosition = this->localPosition = VEC2_ZERO;
            this->globalRotation = this->localRotation = 0.0f;
            this->globalScale = this->localScale = VEC2_ONE;
            this->globalMatrix = this->localMatrix = MAT4_I;
            this->fatherMatrix = MAT4_I;
        }

        Node2D(const glm::vec2 position, const float rotation, const glm::vec2 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalPosition = this->localPosition = position;
            this->globalRotation = this->localRotation = rotation;
            this->globalScale = this->localScale = scale;
            this->globalMatrix = this->localMatrix = this->computeMatrixFromTransform(position, rotation, scale);
            this->fatherMatrix = MAT4_I;
        }

        /** Computes a matrix from the given transform */
        static glm::mat4 computeMatrixFromTransform(glm::vec2 position, float rotation, glm::vec2 scale) {
            return
                glm::translate(MAT4_I, glm::vec3(position, 0.0f)) *
                glm::rotate(MAT4_I, rotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 1.0f)) *
                MAT4_I;
        }

        // Getters
        const glm::vec2& getGlobalPosition() const {
            return this->globalPosition;
        }

        float getGlobalRotation() const {
            return this->globalRotation;
        }

        const glm::vec2& getGlobalScale() const {
            return this->globalScale;
        }

        const glm::mat4& getGlobalMatrix() const {
            return this->globalMatrix;
        }

        /** Returns the direction of the X axis */
        const glm::vec2 getXAxis() const {
            return glm::normalize(glm::vec2(this->globalMatrix[X_ROTATION_INDEX]));
        }

        /** Returns the direction of the Y axis */
        const glm::vec2 getYAxis() const {
            return glm::normalize(glm::vec2(this->globalMatrix[Y_ROTATION_INDEX]));
        }

        const glm::vec2& getLocalPosition() const {
            return this->localPosition;
        }

        float getLocalRotation() const {
            return this->localRotation;
        }

        const glm::vec2& getLocalScale() const {
            return this->localScale;
        }

        const glm::mat4& getLocalMatrix() const {
            return this->localMatrix;
        }

        /** Returns the direction of the local X axis */
        const glm::vec2 getLocalXAxis() const {
            return glm::normalize(glm::vec2(this->localMatrix[X_ROTATION_INDEX]));
        }

        /** Returns the direction of the local Y axis */
        const glm::vec2 getLocalYAxis() const {
            return glm::normalize(glm::vec2(this->localMatrix[Y_ROTATION_INDEX]));
        }

        /** Sets the node's global position */
        void setGlobalPosition(const glm::vec2 position) {
            this->globalMatrix =
                glm::translate(MAT4_I, glm::vec3(position, 0.0f)) *
                glm::translate(MAT4_I, glm::vec3(-this->globalPosition, 0.0f)) *
                this->globalMatrix;
            this->globalPosition = position;

            this->commitGlobalUpdate();
        }

        /** Sets the node's global rotation */
        void setGlobalRotation(const float rotation) {
            this->globalMatrix =
                glm::translate(MAT4_I, glm::vec3(this->globalPosition, 0.0f)) *
                glm::rotate(MAT4_I, rotation, PLANE_ROTATION) *
                glm::rotate(MAT4_I, -this->globalRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->globalPosition, 0.0f)) *
                this->globalMatrix;
            this->globalRotation = rotation;

            this->commitGlobalUpdate();
        }

        /** Sets the node's global scale */
        void setGlobalScale(const glm::vec2 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalMatrix =
                glm::translate(MAT4_I, glm::vec3(this->globalPosition, 0.0f)) *
                glm::rotate(MAT4_I, this->globalRotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 1.0f)) *
                glm::scale(MAT4_I, 1.0f / glm::vec3(this->globalScale, 1.0f)) *
                glm::rotate(MAT4_I, -this->globalRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->globalPosition, 0.0f)) *
                this->globalMatrix;
            this->globalScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f)
                this->updateGlobalTransformPropertiesFromGlobalMatrix();

            this->commitGlobalUpdate();
        }

        /** Moves the node of the given distance, globally */
        void globalTranslate(const glm::vec2 distance) {
            this->globalMatrix = glm::translate(MAT4_I, glm::vec3(distance, 0.0f)) * this->globalMatrix;
            this->globalPosition += distance;

            this->commitGlobalUpdate();
        }

        /** Rotates the node of the given angle, globally */
        void globalRotate(const float angle) {
            this->globalMatrix =
                glm::translate(MAT4_I, glm::vec3(this->globalPosition, 0.0f)) *
                glm::rotate(MAT4_I, angle, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->globalPosition, 0.0f)) *
                this->globalMatrix;
            this->globalRotation += angle;

            this->commitGlobalUpdate();
        }

        /** Scales the node of the given scale, globally */
        void globalScaleAll(const glm::vec2 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalMatrix =
                glm::translate(MAT4_I, glm::vec3(this->globalPosition, 0.0f)) *
                glm::rotate(MAT4_I, this->globalRotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 1.0f)) *
                glm::rotate(MAT4_I, -this->globalRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->globalPosition, 0.0f)) *
                this->globalMatrix;
            this->globalScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f)
                this->updateGlobalTransformPropertiesFromGlobalMatrix();

            this->commitGlobalUpdate();
        }

        /** Sets the node's local position */
        void setLocalPosition(const glm::vec2 position) {
            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(position, 0.0f)) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0.0f)) *
                this->localMatrix;
            this->localPosition = position;

            this->commitLocalUpdate();
        }

        /** Sets the node's local rotation */
        void setLocalRotation(const float rotation) {
            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0.0f)) *
                glm::rotate(MAT4_I, rotation, PLANE_ROTATION) *
                glm::rotate(MAT4_I, -this->localRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0.0f)) *
                this->localMatrix;
            this->localRotation = rotation;

            this->commitLocalUpdate();
        }

        /** Sets the node's local scale */
        void setLocalScale(const glm::vec2 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0.0f)) *
                glm::rotate(MAT4_I, this->localRotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 1.0f)) *
                glm::scale(MAT4_I, 1.0f / glm::vec3(this->localScale, 1.0f)) *
                glm::rotate(MAT4_I, -this->localRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0.0f)) *
                this->localMatrix;
            this->localScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f)
                this->updateLocalTransformPropertiesFromLocalMatrix();

            this->commitLocalUpdate();
        }

        /** Moves the node of the given distance, locally */
        void localTranslate(const glm::vec2 distance) {
            this->localMatrix = glm::translate(MAT4_I, glm::vec3(distance, 0.0f)) * this->localMatrix;
            this->localPosition += distance;

            this->commitLocalUpdate();
        }

        /** Rotates the node of the given angle, locally */
        void localRotate(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0.0f)) *
                glm::rotate(MAT4_I, angle, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0.0f)) *
                this->localMatrix;
            this->localRotation += angle;

            this->commitLocalUpdate();
        }

        /** Scales the node of the given scale, locally */
        void localScaleAll(const glm::vec2 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->localMatrix =
                glm::translate(MAT4_I, glm::vec3(this->localPosition, 0.0f)) *
                glm::rotate(MAT4_I, this->localRotation, PLANE_ROTATION) *
                glm::scale(MAT4_I, glm::vec3(scale, 1.0f)) *
                glm::rotate(MAT4_I, -this->localRotation, PLANE_ROTATION) *
                glm::translate(MAT4_I, glm::vec3(-this->localPosition, 0.0f)) *
                this->localMatrix;
            this->localScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f)
                this->updateLocalTransformPropertiesFromLocalMatrix();

            this->commitLocalUpdate();
        }

        /** Updates the father matrix and all that depends on it (updates global from local) */
        void updateFatherMatrix(glm::mat4 newFatherMatrix) {
            // If father didn't change, do nothing
            if (newFatherMatrix == this->fatherMatrix)
                return;
            this->updateGlobalTransformFromLocal(this, newFatherMatrix);
        }

        /** Returns true if the node has a global reflection */
        bool isReflectedGlobal() const {
            return glm::determinant(this->globalMatrix) < 0.0f;
        }

        /** Returns true if the node has a local reflection */
        bool isReflectedLocal() const {
            return glm::determinant(this->localMatrix) < 0.0f;
        }

        static Node2D* fromJSON(const nlohmann::json& json, Node2D* node = nullptr) {
            Node2D *newNode = node ? node: new Node2D();

            Node::fromJSON(json, newNode);

            glm::vec2 globalPosition = VEC2_ZERO;
            float globalRotation = 0.0f;
            glm::vec2 globalScale = VEC2_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec2(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>());
            if (json.contains("globalRotation")) globalRotation = json["globalRotation"].get<float>();
            if (json.contains("globalScale")) globalScale = glm::vec2(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            return newNode;
        }

    protected:

        /** Commits a global update from the node */
        virtual void commitGlobalUpdate() {

            // Commit update to self
            this->updateLocalMatrixFromGlobal();
            this->updateLocalTransformPropertiesFromLocalMatrix();

            // Propagate to children
            for (Node *child: this->children)
                this->updateGlobalTransformFromLocal(child, this->globalMatrix);
        }

        /** Commits a local update from the node */
        virtual void commitLocalUpdate() {

            // Commit update to self and to the node's children
            this->updateGlobalTransformFromLocal(this, this->fatherMatrix);
        }

    private:

        /** Updates the local matrix from the global and the given the father's matrix */
        void updateLocalMatrixFromGlobal() {
            this->localMatrix = glm::inverse(this->fatherMatrix) * this->globalMatrix;
        }

        /** Updates the local transform from the local matrix */
        void updateLocalTransformPropertiesFromLocalMatrix() {
            this->localPosition = glm::vec2(this->localMatrix[POSITION_INDEX]);

            // Compute the angle
            const glm::vec2 xAxis = this->getLocalXAxis();
            this->localRotation = atan2(xAxis[1], xAxis[0]);

            // Use geometry to compute the scale
            this->localScale = glm::vec2(
                    glm::length(glm::vec2(this->localMatrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec2(this->localMatrix[Y_ROTATION_INDEX]))
                    );
        }

        /** Recursively updates the node and its children and so on */
        static void updateGlobalTransformFromLocal(Node *node, glm::mat4 fatherTransformMatrix) {

            // Update self
            // If node id Node2D, update it, else skip to its children
            if (Node2D* node2d = dynamic_cast<Node2D*>(node)) {
                node2d->fatherMatrix = fatherTransformMatrix;
                node2d->updateGlobalMatrixFromLocal();
                node2d->updateGlobalTransformPropertiesFromGlobalMatrix();
                fatherTransformMatrix = node2d->globalMatrix;
            }

            // Propagate to children
            for (Node *child : node->children) {
                updateGlobalTransformFromLocal(child, fatherTransformMatrix);
            }

        }

        /** Updates the global matrix from the local and the given the father's matrix */
        void updateGlobalMatrixFromLocal() {
            this->globalMatrix = this->fatherMatrix * this->localMatrix;
        }

        /** Updates the global transform from the global matrix */
        void updateGlobalTransformPropertiesFromGlobalMatrix() {
            this->globalPosition = glm::vec2(this->globalMatrix[POSITION_INDEX]);

            // Compute the angle
            const glm::vec2 xAxis = this->getXAxis();
            this->globalRotation = atan2(xAxis[1], xAxis[0]);

            // Use geometry to compute the scale
            this->globalScale = glm::vec2(
                    glm::length(glm::vec2(this->globalMatrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec2(this->globalMatrix[Y_ROTATION_INDEX]))
                    );
        }
};

#endif
