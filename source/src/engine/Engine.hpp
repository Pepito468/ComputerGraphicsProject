// ENGINE
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <sstream>
#include <json.hpp>
#include <vulkan/vulkan_core.h>

#include "Node.hpp"
#include "Node3D.hpp"
#include "Node2D.hpp"

#include "Camera.hpp"

#include "Model3D.hpp"

#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"

#include "UpdateNode3D.hpp"

#include "Renderer.hpp"
#include "common.h"

#define POOL_SIZE 200

class Engine : public BaseProject {

    public:

        /** Static reference to the only Engine */
        inline static Engine *MainEngine = NULL;


    private:

        // Time elapsed from last frame
        float deltaTime = 0.0f;
        // Translation input
        glm::vec3 inputTranslation = VEC3_ZERO;
        // Rotation input
        glm::vec3 inputRotation = VEC3_ZERO;

        /** Root of the current scene */
        Node *scene;

        /** Main camera (camera that is being used to render) */
        Camera *mainCamera;



    public:

        Engine() : renderer(this,
                &RP,
                [this]() {submitCommandBuffer("main", 0, populateCommandBufferAccess, this); }
                ) {}

        /** Global getters */
        float getDeltaTime() const {
            return this->deltaTime;
        }

        const glm::vec3 getInputTranslation() const {
            return this->inputTranslation;
        }

        const glm::vec3 getInputRotation() const {
            return this->inputRotation;
        }


        /** Returns true if the given key is being pressed */
        bool isKeyBeingPressed(int key) {
            return glfwGetKey(this->window, key);
        }

        /** Sets the given node as the current scene */
        void setSceneRoot(Node *scene) {
            this->scene = scene;
        }

        /** Sets the given camera as main camera */
        void setMainCamera(Camera *camera) {
            this->mainCamera = camera;
        }

        /** Shuts down the Engine and closes the window */
        void shutdown() {
            glfwSetWindowShouldClose(this->window, GL_TRUE);
        }

    private:

        /** Recomputes the Node3D hierarchy */
        void recompute3DNodeHierarchy(Node* node, glm::mat4 fatherTransformMatrix) {

            // If a Node3D is found, propagate an update to it
            if (Node3D* node3d = dynamic_cast<Node3D*>(node)) {
                node3d->updateFatherMatrix(fatherTransformMatrix);
                return;
            }

            // Else try again with the children
            for (Node *child : node->children)
                recompute3DNodeHierarchy(child, fatherTransformMatrix);
        }

        /** Recomputes the matrices after the Node3Ds are moved and the hierarchy has changed */
        void recompute2DNodeHierarchy(Node* node, glm::mat4 fatherTransformMatrix) {

            // If the node is Node2D, propagate the update
            if (Node2D* node2d = dynamic_cast<Node2D*>(node)) {
                node2d->updateFatherMatrix(fatherTransformMatrix);
                return;
            }

            // Else continue looking through the children
            for (Node *child : node->children)
                recompute2DNodeHierarchy(child, fatherTransformMatrix);
        }

        /** Calls update() on the given node and its descendants */
        void updateUpdate3DNodes(Node *node) {
            if (UpdateNode3D *updateNode = dynamic_cast<UpdateNode3D*>(node))
                updateNode->update();

            for (Node *child : node->children)
                updateUpdate3DNodes(child);
        }

        void loadScene(Node *node) {
            // Call onEnter()
            node->onEnter(); // TODO: maybe node spawning can be handled in a different place or way

            // Node specific initialization
            if (Model3D *model = dynamic_cast<Model3D*>(node)) {
                renderer.instantiate(model->getModelPath(), model->getGlobalPosition(), model->getGlobalRotation(), model->getGlobalScale(), model->getMaterial(), false);
            } else if (DirectionalLight *light = dynamic_cast<DirectionalLight*>(node)) {
                renderer.createDirectionalLight(light->radiance, light->color, light->getGlobalRotation());
            } else if (AmbientLight *light = dynamic_cast<AmbientLight*>(node)) {
                renderer.setAmbientLight(light->upper, light->lower, light->dir);
            } else if (PointLight *light = dynamic_cast<PointLight*>(node)) {
                renderer.addPointLight(light->getGlobalPosition(), light->radiance, light->color, light->radius, light->decay, light->isOn);
            } else if (SpotLight *light = dynamic_cast<SpotLight*>(node)) {
                renderer.addSpotLight(light->getGlobalPosition(), light->radiance, light->color, light->aperture, light->decay, light->getGlobalRotation());
            }

            log(std::format("Loaded Node [{}, ID: {}]", node->name, node->UUID));
            for (Node *child : node->children)
                loadScene(child);
        }

        void engineInit() {
            info("Starting Engine");

            // Engine checks
            if (!scene)
                error("Scene not set");
            if (!mainCamera)
                warning("Main camera not set");

            // Set static reference
            Engine::MainEngine = this;

            // Load scene
            info("Loading Scene");
            this->loadScene(this->scene);
            info("Succesfully loaded scene");

        }

        bool isFirstEngineLoop = true;

        void engineLoop() {
            if (this->isFirstEngineLoop) {
                this->engineInit();
                this->isFirstEngineLoop = false;
            }

            // Update globals
            bool fire = false; // Maybe we can use this
            this->inputTranslation = VEC3_ZERO;
            this->inputRotation = VEC3_ZERO;
            getSixAxis(this->deltaTime, this->inputTranslation, this->inputRotation, fire);

            // Call update() on all UpdateNodes
            this->updateUpdate3DNodes(this->scene);

            // Recompute hierarchy in case something changed
            this->recompute3DNodeHierarchy(this->scene, MAT4_I);
            this->recompute2DNodeHierarchy(this->scene, MAT4_I);

            // TODO: move txt to its node

            // updates the FPS
            static float elapsedT = 0.0f;
            static int countedFrames = 0;

            countedFrames++;
            elapsedT += this->deltaTime;
            if(elapsedT > 1.0f) {
                float Fps = (float)countedFrames / elapsedT;

                std::ostringstream oss;
                oss << "FPS: " << Fps << "\n";

                txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});

                elapsedT = 0.0f;
                countedFrames = 0;
            }

        }



    protected:
        // Here you list all the Vulkan objects you need:
        Renderer renderer;
        RenderPass RP;

        // to provide textual feedback
        TextMaker txt;

        void localInit() {

            // renderer.loadSceneFromJSON();
            renderer.localInit();

            // initializes the render passes
            RP.init(this);

            // sets the blue sky
            RP.properties[0].clearValue = {0.0f,0.0f,0.0f,0.0f};


            // sets the size of the Descriptor Set Pool (it MUST be done before loading the scene)
            DPSZs.uniformBlocksInPool = POOL_SIZE;
            DPSZs.texturesInPool = POOL_SIZE;
            DPSZs.setsInPool = POOL_SIZE;

            // initializes the textual output
            txt.init(this, windowWidth, windowHeight);

            // submits the main command buffer
            submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

            // Prepares for showing the FPS count
            txt.print(1.0f, 1.0f, "FPS:",1,"CO",false,false,true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});
            txt.print(-1.0f, -1.0f ,  "Testo di prova", 2, "CO", false, false, false, TAL_LEFT, TRH_LEFT, TRV_TOP, {0.5f, 0.5f, 0.0f, 0.5f}, {0.5f,0.5f,0.0f,0.5f});

        }

        // Here you create your pipelines and Descriptor Sets!
        void pipelinesAndDescriptorSetsInit() {
            // creates the render passes
            RP.create();

            renderer.descriptorSetsInits();

            txt.pipelinesAndDescriptorSetsInit();
        }

        // Here you destroy your pipelines and Descriptor Sets!
        void pipelinesAndDescriptorSetsCleanup() {
            RP.cleanup();
            renderer.descriptorSetsCleanup();
            txt.pipelinesAndDescriptorSetsCleanup();
        }

        // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
        // You also have to destroy the pipelines
        void localCleanup() {
            RP.destroy();

            renderer.localCleanup();

            txt.localCleanup();
        }

        // Here it is the creation of the command buffer:
        // You send to the GPU all the objects you want to draw,
        // with their buffers and textures
        static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
            // Simple trick to avoid having always 'T->'
            // in che code that populates the command buffer!
            Engine *T = (Engine *)Params;
            T->populateCommandBuffer(commandBuffer, currentImage);
        }

        void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

            renderer.populateShadowCommandBuffer(commandBuffer, currentImage);

            // Offscreen pass - always required
            // begin standard pass
            RP.begin(commandBuffer, currentImage);

            renderer.populateCommandBuffer(commandBuffer, currentImage);

            RP.end(commandBuffer);
        }

        // Here is where you update the uniforms.
        // Very likely this will be where you will be writing the logic of your application.
        void updateUniformBuffer(uint32_t currentImage) {

            // Engine logic
            this->engineLoop();


            // Renderer updates
            // Update with camera data
            renderer.updateUniformBuffer(currentImage,
                    this->mainCamera->getGlobalPosition(),
                    this->mainCamera->getProjectionMatrix(),
                    this->mainCamera->getViewMatrix());

            renderer.updateLightCulling(this->mainCamera->getGlobalPosition(), 10.0f);

            txt.updateCommandBuffer();
        }

        // Here you set the main application parameters
        void setWindowParameters() {

            // window size, titile and initial background
            windowWidth = 1080;
            windowHeight = 720;
            windowTitle = "VIDEOGAME";
            windowResizable = GLFW_TRUE;
        }

        // What to do when the window changes size
        void onWindowResize(int w, int h) {

            log(std::format("Window resized to {} x {}", w, h));

            // Update Render Pass
            RP.width = w;
            RP.height = h;

            // updates the textual output
            txt.resizeScreen(w, h);
        }

};

#endif
