// 3DNode
#ifndef ENGINE_NODE3D_H
#define ENGINE_NODE3D_H

#define GLM_FORCE_RADIANS

#include "Debug.hpp"
#include "Node.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include <format>
#include <glm/gtc/epsilon.hpp>
#include <glm/vector_relational.hpp>
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
#define DEFAULT_POINT_SCALE 1
#define NODE3D_EPSILON 0.001f

/**
 * A Node that has a place in 3D space
 *
 * NOTES:
 *      the rotation convention is YXZ
 *      the rotation angles are in radians
 *
 */
class Node3D : public Node {

    private:

        /// Node's transformation matrix
        glm::mat4 globalMatrix;
        glm::vec3 globalPosition;
        glm::vec3 globalRotation;
        glm::vec3 globalScale;

        /// Father's matrix
        glm::mat4 fatherMatrix;

        /// Node's local matrix
        glm::mat4 localMatrix;
        glm::vec3 localPosition;
        glm::vec3 localRotation;
        glm::vec3 localScale;


    public:

        /// Default constructor
        Node3D() : Node() {
            this->globalPosition = this->localPosition = VEC3_ZERO;
            this->globalRotation = this->localRotation = VEC3_ZERO;
            this->globalScale = this->localScale = VEC3_ONE;
            this->globalMatrix = this->localMatrix = MAT4_I;
            this->fatherMatrix = MAT4_I;
        }

        /// Constructor with parameters
        Node3D(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node() {
            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
                // A scale of 0 produces nan after a rotation
                error(std::format("Trying to set scale for [{}] to 0", this->UUID));
            }
            // Check for shear
            if (scale.x != scale.y || scale.x != scale.z || scale.y != scale.z) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalPosition = this->localPosition = position;
            this->globalRotation = this->localRotation = rotation;
            this->globalScale = this->localScale = scale;
            this->globalMatrix = this->localMatrix = computeMatrixFromTransform(position, rotation, scale);
            this->fatherMatrix = MAT4_I;
        }

        /** Computes the matrix from position, rotation and scale */
        static glm::mat4 computeMatrixFromTransform(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) {
            return
                glm::translate(MAT4_I, position) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                MAT4_I;
        }

        // Getters
        const glm::vec3& getGlobalPosition() const {
            return this->globalPosition;
        }

        const glm::vec3& getGlobalRotation() const {
            return this->globalRotation;
        }

        const glm::vec3& getGlobalScale() const {
            return this->globalScale;
        }

        const glm::mat4& getGlobalMatrix() const {
            return this->globalMatrix;
        }

        /// Gets the node's X-axis
        const glm::vec3 getXAxis() const {
            return glm::normalize(glm::vec3(this->globalMatrix[X_ROTATION_INDEX]));
        }

        /// Gets the node's Y-axis
        const glm::vec3 getYAxis() const {
            return glm::normalize(glm::vec3(this->globalMatrix[Y_ROTATION_INDEX]));
        }

        /// Gets the node's Z-axis
        const glm::vec3 getZAxis() const {
            return glm::normalize(glm::vec3(this->globalMatrix[Z_ROTATION_INDEX]));
        }

        const glm::vec3& getLocalPosition() const {
            return this->localPosition;
        }

        const glm::vec3& getLocalRotation() const {
            return this->localRotation;
        }

        const glm::vec3& getLocalScale() const {
            return this->localScale;
        }

        const glm::mat4& getLocalMatrix() const {
            return this->localMatrix;
        }

        /// Gets the node's local X-axis
        const glm::vec3 getLocalXAxis() const {
            return glm::normalize(glm::vec3(this->localMatrix[X_ROTATION_INDEX]));
        }

        /// Gets the node's local Y-axis
        const glm::vec3 getLocalYAxis() const {
            return glm::normalize(glm::vec3(this->localMatrix[Y_ROTATION_INDEX]));
        }

        /// Gets the node's local Z-axis
        const glm::vec3 getLocalZAxis() const {
            return glm::normalize(glm::vec3(this->localMatrix[Z_ROTATION_INDEX]));
        }

        /** Sets the node's global position */
        void setGlobalPosition(const glm::vec3 position) {
            this->globalMatrix = 
                glm::translate(MAT4_I, position) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalPosition = position;

            this->commitGlobalUpdate();
        }

        /** Sets the node's global rotation */
        void setGlobalRotation(const glm::vec3 rotation) {
            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->globalRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->globalRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalRotation = rotation;

            this->commitGlobalUpdate();
        }

        /** Sets the node's global scale */
        void setGlobalScale(const glm::vec3 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear check
            if (scale.x != scale.y || scale.x != scale.z || scale.y != scale.z) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, this->globalRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->globalRotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                glm::scale(MAT4_I, 1.0f / this->globalScale) *
                glm::rotate(MAT4_I, -this->globalRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->globalRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalScale = scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f)
                this->updateGlobalTransformPropertiesFromGlobalMatrix();

            this->commitGlobalUpdate();
        }

        /// Sets the node's global matrix
        void setGlobalMatrix(glm::mat4 mat)
        {
            globalMatrix = mat;
            updateGlobalTransformPropertiesFromGlobalMatrix();
            commitGlobalUpdate();
        }

        /// Translates the node globally
        void globalTranslate(const glm::vec3 distance) {
            this->globalMatrix = glm::translate(MAT4_I, distance) * this->globalMatrix;
            this->globalPosition += distance;

            this->commitGlobalUpdate();
        }

        /// Rotates the node around the global X-axis parallel
        void globalRotateX(const float angle) {
            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, this->globalRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, angle, VEC3_X) *
                glm::rotate(MAT4_I, -this->globalRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalRotation.x += angle;

            this->commitGlobalUpdate();
        }

        /// Rotate the node around the global Y-axis parallel
        void globalRotateY(const float angle) {
            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, angle, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalRotation.y += angle;

            this->commitGlobalUpdate();
        }

        /// Rotate the node around the global Z-axis parallel
        void globalRotateZ(const float angle) {
            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, this->globalRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, angle, VEC3_Z) *
                glm::rotate(MAT4_I, -this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->globalRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalRotation.z += angle;

            this->commitGlobalUpdate();
        }

        /**
         * Scales the node by the given amount, globally
         * */
        void globalScaleAll(const glm::vec3 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear check
            if (scale.x != scale.y || scale.x != scale.z || scale.y != scale.z) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->globalMatrix =
                glm::translate(MAT4_I, this->globalPosition) *
                glm::rotate(MAT4_I, this->globalRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->globalRotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                glm::rotate(MAT4_I, -this->globalRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->globalRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->globalRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->globalPosition) *
                this->globalMatrix;
            this->globalScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f)
                this->updateGlobalTransformPropertiesFromGlobalMatrix();

            this->commitGlobalUpdate();
        }

        /** Sets the node's local position */
        void setLocalPosition(const glm::vec3 position) {
            this->localMatrix = 
                glm::translate(MAT4_I, position) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localPosition = position;

            this->commitLocalUpdate();
        }

        /** Sets the node's local rotation */
        void setLocalRotation(const glm::vec3 rotation) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, rotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, rotation.x, VEC3_X) *
                glm::rotate(MAT4_I, rotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation = rotation;

            this->commitLocalUpdate();
        }

        /** Sets the node's local scale */
        void setLocalScale(const glm::vec3 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear check
            if (scale.x != scale.y || scale.x != scale.z || scale.y != scale.z) {
                warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->localRotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                glm::scale(MAT4_I, 1.0f / this->localScale) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localScale = scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f)
                this->updateLocalTransformPropertiesFromLocalMatrix();

            this->commitLocalUpdate();
        }

        /// Translates the node locally
        void localTranslate(const glm::vec3 distance) {
            this->localMatrix = glm::translate(MAT4_I, distance) * this->localMatrix;
            this->localPosition += distance;

            this->commitLocalUpdate();
        }

        /// Rotates the node around the local X-axis parallel
        void localRotateX(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, angle, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.x += angle;

            this->commitLocalUpdate();
        }

        /// Rotate the node around the local Y-axis parallel
        void localRotateY(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, angle, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.y += angle;

            this->commitLocalUpdate();
        }

        /// Rotate the node around the local Z-axis parallel
        void localRotateZ(const float angle) {
            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, angle, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localRotation.z += angle;

            this->commitLocalUpdate();
        }

        /**
         * Scales the node by the given amount, locally
         * */
        void localScaleAll(const glm::vec3 scale) {
            // Scale cannot be 0
            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f)
                error(std::format("Flattening, at leat one of the scaling factors for [{}] is 0", this->UUID));
            // Shear might happen
            if (scale.x != scale.y || scale.x != scale.z || scale.y != scale.z) {
               warning(std::format("Non uniform scale being applied to [{}]. This may cause shear to its children", this->UUID));
            }

            this->localMatrix =
                glm::translate(MAT4_I, this->localPosition) *
                glm::rotate(MAT4_I, this->localRotation.y, VEC3_Y) *
                glm::rotate(MAT4_I, this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, this->localRotation.z, VEC3_Z) *
                glm::scale(MAT4_I, scale) *
                glm::rotate(MAT4_I, -this->localRotation.z, VEC3_Z) *
                glm::rotate(MAT4_I, -this->localRotation.x, VEC3_X) *
                glm::rotate(MAT4_I, -this->localRotation.y, VEC3_Y) *
                glm::translate(MAT4_I, -this->localPosition) *
                this->localMatrix;
            this->localScale *= scale;

            // Handle reflections
            if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f)
                this->updateLocalTransformPropertiesFromLocalMatrix();

            this->commitLocalUpdate();
        }

        /** Updates the father matrix and all that depends on it (updates global from local) */
        void updateFatherMatrix(glm::mat4 newFatherMatrix) {
            this->updateGlobalTransformFromLocal(this, newFatherMatrix);
        }

        /// Computes the local coordinates of the given point from the node's transform
        glm::vec3 toLocalSpace(const glm::vec3 point) const {
            return glm::vec3(glm::inverse(this->globalMatrix) * glm::vec4(point, DEFAULT_POINT_SCALE));
        }

        /// Computes the global coordinates of the given point
        glm::vec3 toGlobalSpace(const glm::vec3 point) const {
            return glm::vec3(this->globalMatrix * glm::vec4(point, DEFAULT_POINT_SCALE));
        }

        /** Returns true if the node has a global reflection */
        bool isReflectedGlobal() const {
            return glm::determinant(this->globalMatrix) < 0.0f;
        }

        /** Returns true if the node has a local reflection */
        bool isReflectedLocal() const {
            return glm::determinant(this->localMatrix) < 0.0f;
        }

        static Node3D* fromJSON(const nlohmann::json& json) {
            Node3D *newNode = new Node3D();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            return newNode;
        }

    protected:

        /** Commits a local update from the node */
        virtual void commitLocalUpdate() {

            // Commit update to self and to the node's children
            this->updateGlobalTransformFromLocal(this, this->fatherMatrix);
        }

        /** Commits a global update from the node */
        virtual void commitGlobalUpdate() {

            // Commit update to self
            this->updateLocalMatrixFromGlobal();
            this->updateLocalTransformPropertiesFromLocalMatrix();

            // Propagate to children (updates local since their global doesn't change, father does)
            for (Node *child : this->children)
                this->updateGlobalTransformFromLocal(child, this->globalMatrix);
        }

    private:

        /**
         *  Updates the local matrix from the global
         */
        void updateLocalMatrixFromGlobal() {
            this->localMatrix = glm::inverse(this->fatherMatrix) * this->globalMatrix;
        }

        /**
         *  Updates the local transform properties of the node from the local matrix (position, rotation, scale).
         */
        void updateLocalTransformPropertiesFromLocalMatrix() {

            // Position
            this->localPosition = glm::vec3(this->localMatrix[POSITION_INDEX]);

            // Rotation
            // Compute rotation from the rotation matrix (the local axes) using geometry
            const glm::vec3 xAxis = this->getLocalXAxis();
            const glm::vec3 yAxis = this->getLocalYAxis();
            const glm::vec3 zAxis = this->getLocalZAxis();
            if (glm::epsilonEqual(std::abs(zAxis[1]), 1.0f, NODE3D_EPSILON)) {
                warning(std::format("Gimbal Lock on [{}] (local)", this->UUID));
            }
            this->localRotation =  glm::vec3(
                    std::asin(-zAxis[1]),
                    std::atan2(zAxis[0], zAxis[2]),
                    std::atan2(xAxis[1], yAxis[1])
                    );

            // Scale
            // Use Pythagoras' theorem to get the length of the axis
            this->localScale = glm::vec3(
                    glm::length(glm::vec3(this->localMatrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->localMatrix[Y_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->localMatrix[Z_ROTATION_INDEX]))
                    );

        }

        /** Recursively updates the node and its children and so on */
        static void updateGlobalTransformFromLocal(Node *node, glm::mat4 fatherTransformMatrix) {

            // Update self
            // If node id Node3D, update it, else skip to its children
            if (Node3D* node3d = dynamic_cast<Node3D*>(node)) {
                node3d->fatherMatrix = fatherTransformMatrix;
                node3d->updateGlobalMatrixFromLocal();
                node3d->updateGlobalTransformPropertiesFromGlobalMatrix();
                fatherTransformMatrix = node3d->globalMatrix;
            }

            // Propagate to children
            for (Node *child : node->children) {
                updateGlobalTransformFromLocal(child, fatherTransformMatrix);
            }

        }

        /**
         *  Updates the global matrix from the local
         * */
        void updateGlobalMatrixFromLocal() {
            this->globalMatrix = this->fatherMatrix * this->localMatrix;
        }

        /**
         *  Updates the global transform properties of the node from the global matrix (position, rotation, scale).
         * */
        void updateGlobalTransformPropertiesFromGlobalMatrix() {

            // Position
            this->globalPosition = glm::vec3(this->globalMatrix[POSITION_INDEX]);

            // Rotation
            // Compute rotation from the rotation matrix (the local axes) using geometry
            const glm::vec3 xAxis = this->getXAxis();
            const glm::vec3 yAxis = this->getYAxis();
            const glm::vec3 zAxis = this->getZAxis();
            if (glm::epsilonEqual(std::abs(zAxis[1]), 1.0f, NODE3D_EPSILON)) {
                warning(std::format("Gimbal Lock on [{}] (global)", this->UUID));
            }
            this->globalRotation =  glm::vec3(
                    std::asin(-zAxis[1]),
                    std::atan2(zAxis[0], zAxis[2]),
                    std::atan2(xAxis[1], yAxis[1])
                    );

            // Scale
            // Use Pythagoras' theorem to get the length of the axis
            this->globalScale = glm::vec3(
                    glm::length(glm::vec3(this->globalMatrix[X_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->globalMatrix[Y_ROTATION_INDEX])),
                    glm::length(glm::vec3(this->globalMatrix[Z_ROTATION_INDEX]))
                    );

        }

};

/// Specialization to make Node3D compatible with std::format
template <>
struct std::formatter<Node3D> : std::formatter<Node> {};

/// Specialization to make Node3D* compatible with std::format
template <>
struct std::formatter<Node3D*> : std::formatter<Node*> {};
#endif
