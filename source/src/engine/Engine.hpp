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
#include "Physics.hpp"
#include "UIMaker.hpp"

#define POOL_SIZE 200
#define LIGHT_RENDER_DISTANCE 20.0f
#define DEFAULT_CURSOR GLFW_CURSOR_NORMAL

class Engine : public BaseProject {

    public:
        /** Static reference to the only Engine */
        inline static Engine *MainEngine = NULL;


    private:

        // Time elapsed from last frame
        float deltaTime = 0.0f;
        // Time elapsed since the start
        float time;
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
                ), ui(this) {

            // Set static reference to this
            Engine::MainEngine = this;
        }

        /** Global getters */
        static float getDeltaTime() {
            return MainEngine->deltaTime;
        }

        static glm::vec3 getInputTranslation() {
            return MainEngine->inputTranslation;
        }

        static glm::vec3 getInputRotation()
        {
            return MainEngine->inputRotation;
        }

        /** Returns true if the given key is being pressed */
        static bool isKeyBeingPressed(int key) {
            return glfwGetKey(MainEngine->window, key);
        }

        /** Sets the given node as the current scene */
        static void setSceneRoot(Node *scene) {
            MainEngine->scene = scene;
        }

        /** Sets the given camera as main camera */
        static void setMainCamera(Camera *camera) {
            MainEngine->mainCamera = camera;
        }

        /** Shuts down the Engine and closes the window */
        static void shutdown() {
            glfwSetWindowShouldClose(MainEngine->window, GL_TRUE);
        }

        /** Sets the cursor mode to the one given */
        static void setCursorMode(int mode) {
            glfwSetInputMode(MainEngine->window, GLFW_CURSOR, mode);
        }

        /** Returns the current cursor mode */
        static int getCursorMode() {
            return glfwGetInputMode(MainEngine->window, GLFW_CURSOR);
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
                renderer.instantiate(model, false);
            } else if (DirectionalLight *light = dynamic_cast<DirectionalLight*>(node)) {
                renderer.createDirectionalLight(light);
            } else if (AmbientLight *light = dynamic_cast<AmbientLight*>(node)) {
                renderer.setAmbientLight(light);
            } else if (PointLight *light = dynamic_cast<PointLight*>(node)) {
                renderer.addPointLight(light);
            } else if (SpotLight *light = dynamic_cast<SpotLight*>(node)) {
                renderer.addSpotLight(light);
            }

            log(std::format("Loaded Node [{}, ID: {}]", node->name, node->UUID));
            for (Node *child : node->children)
                loadScene(child);
        }

        /** Starts the Engine */
        void engineInit() {
            info("Starting Engine");

            // Engine checks
            if (!scene)
                error("Scene not set");
            if (!mainCamera)
                warning("Main camera not set");

            // Set cursor
            Engine::setCursorMode(DEFAULT_CURSOR);

            // Load scene
            info("Loading Scene");
            this->loadScene(this->scene);
            Physics::loadScene(this->scene);
            info("Succesfully loaded scene");
        }

        bool isFirstEngineLoop = true;

        void engineLoop() {
            if (this->isFirstEngineLoop) {
                this->engineInit();
                this->isFirstEngineLoop = false;
            }

            // Read inputs
            this->getSixAsixFixed(this->deltaTime, this->inputTranslation, this->inputRotation);

            // Update time
            this->time += this->deltaTime;

            // Call update() on all UpdateNodes
            this->updateUpdate3DNodes(this->scene);

            // Recompute hierarchy in case something changed
            this->recompute3DNodeHierarchy(this->scene, MAT4_I);
            this->recompute2DNodeHierarchy(this->scene, MAT4_I);

            // Physics checks
            Physics::checkCollisions();


            // TODO: move txt to its node

            // updates the FPS
            static float elapsedT = 0.0f;
            static int countedFrames = 0;
            static float loadingBarSize = 1.0f;
            static int direction = 1;

            countedFrames++;
            elapsedT += this->deltaTime;
            if(elapsedT > 1.0f) {
                float Fps = (float)countedFrames / elapsedT;

                std::ostringstream oss;
                oss << "FPS: " << Fps << "\n";

                txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});

                elapsedT = 0.0f;
                countedFrames = 0;

                loadingBarSize += 1.0f * direction;
                if (loadingBarSize >= 10.0f || loadingBarSize <= 1.0f) {
                    direction = -1 * direction;
                }
                ui.renderUI(-0.95f, 0.95f, 0, UIO_LEFT, UIO_BOTTOM, loadingBarSize, 1.0f);
            }

        }

        /// Wrapper for input detection
        float getSixAsixFixed(float &deltaT, glm::vec3 &m, glm::vec3 &r) {
            // Reset values
            m = VEC3_ZERO;
            r = VEC3_ZERO;
            deltaT = 0.0f;
            bool fire; // Won't be used

            // Read values from Starter
            getSixAxis(deltaT, m, r, fire);

            // Capturing the rotation with no 'click' constraint from 'getSixAsix()' in Starter
            static double old_xpos = 0, old_ypos = 0;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            double m_dx = xpos - old_xpos;
            double m_dy = ypos - old_ypos;
            old_xpos = xpos; old_ypos = ypos;
            const float MOUSE_RES = 10.0f;
            glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
            r.y = m_dx / MOUSE_RES;
            r.x = m_dy / MOUSE_RES;

            return deltaT;
        }


    protected:
        // Here you list all the Vulkan objects you need:
        Renderer renderer;
        RenderPass RP;

        // to provide textual feedback
        TextMaker txt;

        UIMaker ui;

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
            ui.init(windowWidth, windowHeight, {}, {
                {ProceduralTextures::generateTexture(16, 16, 255, 0, 0)},
                {ProceduralTextures::generateTexture(16, 16, 0, 255, 255), ProceduralTextures::generateTexture(16, 16, 0, 0, 255)},
            });

            // submits the main command buffer
            submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

            // Prepares for showing the FPS count
            txt.print(1.0f, 1.0f, "FPS:",1,"CO",false,false,true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});
            txt.print(-1.0f, -1.0f ,  "Testo di prova", 2, "CO", false, false, false, TAL_LEFT, TRH_LEFT, TRV_TOP, {0.5f, 0.5f, 0.0f, 0.5f}, {0.5f,0.5f,0.0f,0.5f});

            ui.renderUI(-0.95f, 0.95f, 0, UIO_LEFT, UIO_BOTTOM, 1.0f, 1.0f);
            ui.renderUI(-1.0f, 1.0f, 1, UIO_LEFT, UIO_BOTTOM, 5.0f, 5.0f);
        }

        // Here you create your pipelines and Descriptor Sets!
        void pipelinesAndDescriptorSetsInit() {
            // creates the render passes
            RP.create();

            renderer.descriptorSetsInits();

            txt.pipelinesAndDescriptorSetsInit();
            ui.pipelinesAndDescriptorSetsInit();
        }

        // Here you destroy your pipelines and Descriptor Sets!
        void pipelinesAndDescriptorSetsCleanup() {
            RP.cleanup();
            renderer.descriptorSetsCleanup();
            txt.pipelinesAndDescriptorSetsCleanup();
            ui.pipelinesAndDescriptorSetsCleanup();
        }

        // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
        // You also have to destroy the pipelines
        void localCleanup() {
            RP.destroy();

            renderer.localCleanup();

            txt.localCleanup();
            ui.localCleanup();
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
            renderer.populateSceneDepthCommandBuffer(commandBuffer, currentImage);
            renderer.populateSceneColorCommandBuffer(commandBuffer, currentImage);

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
                    this->mainCamera->getViewMatrix(), time);

            renderer.updateLightCulling(this->mainCamera->getGlobalPosition(), LIGHT_RENDER_DISTANCE);

            txt.updateCommandBuffer();
            ui.updateCommandBuffer();
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
            ui.resizeScreen(w, h);
        }

};

#endif
