// ENGINE
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <sstream>
#include <json.hpp>

#include "Debug.hpp"

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
#include "Physics.hpp"

#include "UIMaker.hpp"

#include "AudioNode.hpp"

#include "common.h"

#define POOL_SIZE 1000
#define LIGHT_RENDER_DISTANCE 20.0f
#define DEFAULT_CURSOR GLFW_CURSOR_NORMAL
#define AUDIO_LISTENER 0

class Engine : public BaseProject {

    public:
        /** Static reference to the only Engine */
        inline static Engine *MainEngine = nullptr;


    private:

        // Time elapsed from last frame
        float deltaTime = 0.0f;
        // Time elapsed since the start
        float time = 0.0f;
        // Translation input
        glm::vec3 inputTranslation = VEC3_ZERO;
        // Rotation input
        glm::vec3 inputRotation = VEC3_ZERO;
        // Keys that are currently registered as pressed down
        std::set<int> currentlyPressedKeys = {};

        /** Root of the current scene */
        Node *scene = nullptr;

        /** Main camera (camera that is being used to render) */
        Camera *mainCamera = nullptr;

        /** This holds the value of a scene that has been requested to be loaded by someone. It will be loaded at the next Engine loop */
        Node *sceneToLoad = nullptr;

        std::set<Node*> nodesQueuedToBeDeleted;

        std::map<std::string, Node*> scenes;

        ma_engine audioEngine;

        /// Stores the cursor position
        double cursorPosX = 0, cursorPosY = 0, oldCursorPosX = 0, oldCursorPosY = 0;

        bool shutdownRequested = false;

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

        static glm::vec3 getInputRotation() {
            return MainEngine->inputRotation;
        }

        /** Returns true if the given key is being pressed */
        static bool isKeyBeingPressed(const int key, const bool onlyOnPress = false) {
            // glfwGetKey doesn't work with mouse buttons
            if (glfwGetKey(MainEngine->window, key) || glfwGetMouseButton(MainEngine->window, key) == GLFW_PRESS) {
                const bool wasPressed = MainEngine->currentlyPressedKeys.contains(key);
                MainEngine->currentlyPressedKeys.insert(key);
                return !onlyOnPress || !wasPressed;
            }

            MainEngine->currentlyPressedKeys.erase(key);
            return false;
        }

        static void setMainCamera(Camera *camera) {
            if (!camera)
                error("Trying to set a NULL camera");

            MainEngine->mainCamera = camera;
            log(std::format("Main camera changed to [{}, ID: {}]", camera->name, camera->UUID));
        }

        /** Shuts down the Engine */
        static void requestEngineShutdown() {
            MainEngine->shutdownRequested = true;
        }

        /** Sets the cursor mode to the one given */
        static void setCursorMode(int mode) {
            glfwSetInputMode(MainEngine->window, GLFW_CURSOR, mode);
        }

        /** Returns the current cursor mode */
        static int getCursorMode() {
            return glfwGetInputMode(MainEngine->window, GLFW_CURSOR);
        }

        /** Returns true if the pause menu is open (ie. the cursor is in normal mode) */
        static bool isPauseMenuOpen() {
            return (getCursorMode() == GLFW_CURSOR_NORMAL);
        }

        /** Communicates to the ui that something was clicked in the menu */
        static void handleMenuClick() {
            double mouseX, mouseY; //TODO I didn't bother changing the global cursorPos[X/Y] since updateMouseStatus is gonna get called anyway. If other functions use the global variable, this probably should update them
            glfwGetCursorPos(MainEngine->window, &mouseX, &mouseY);
            MainEngine->ui.updateMouseStatus(mouseX, mouseY, true);
        }

        /** Returns the scene's root */
        static Node* getSceneRoot() {
            return MainEngine->scene;
        }

        /** Changes the loaded scene to the new one (given the root).
         *  NOTE: the scene change will happen in the next Engine Loop
         * */
        static void requestSceneChange(Node *newRoot) {
            if (!newRoot) {
                warning("Requesting a NULL scene does nothing");
                return;
            }

            if (newRoot == MainEngine->scene) {
                warning("Trying to load the current scene again");
                return;
            }

            MainEngine->sceneToLoad = newRoot;
            log(std::format("Scene change request accepted [to {}, ID: {}]", newRoot->name, newRoot->UUID));
        }

        /** Toggles visibility of the pause menu */
        static void togglePauseMenu() {
            //TODO use #define for ids
            MainEngine->ui.toggleVisibility(UI_ID_PAUSE_MENU_BACKGROUND);
            MainEngine->ui.toggleVisibility(UI_ID_BUTTON);
        }

        /** Maps a name to a scene (for convenience).
         * @note deleting a scene does not delete its mapping from here.
         * */
        static void mapScene(const std::string& sceneName, Node* sceneRoot) {
            MainEngine->scenes[sceneName] = sceneRoot;
        }

        /** Returns the scene mapped with its name (or nullptr if no scene was mapped with that name) */
        static Node* getSceneFromNameMap(const std::string& sceneName) {
            return MainEngine->scenes[sceneName];
        }

        /**
         * Loads a node and its descendants into the scene instantly.
         * @param node The root node to add.
         * @param parent The parent of the node. Defaults to the scene root.
         */
        static void instantiate(Node* node, Node* parent = MainEngine->scene) {
            if (!node || !parent)
                error("Can't instantiate NULL node or have NULL parent");

            parent->adopt(node);
            MainEngine->addNodeRecursive(node);
        }

        /**
         * Queue a node for deletion.
         * @param deleteDescendants flag to specify if the node's descendants have to be deleted (default = false).
         * @note The node will actually be deleted in the next Engine loop.
         */
        static void requestNodeDeletion(Node *node, const bool deleteDescendants = false) {
            if (!node)
                error("Deleting NULL node");

            if (node == MainEngine->scene)
                error("Can't free the scene root, request a scene change instead");

            if (MainEngine->nodesQueuedToBeDeleted.contains(node))
                warning(std::format("Requesting deletion for the same node again: [{}, ID: {}]", node->name, node->UUID));


            MainEngine->nodesQueuedToBeDeleted.insert(node);
            if (deleteDescendants)
                for (Node *child : node->children)
                    requestNodeDeletion(child, true);

            log(std::format("Node deletion request accepted [{}, ID: {}], delete children? [{}]", node->name, node->UUID, deleteDescendants ? "Y" : "N"));
        }

    private:

        /** Recomputes the Node3D hierarchy */
        void recompute3DNodeHierarchy(Node* node, const glm::mat4& fatherTransformMatrix) {

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
        void recompute2DNodeHierarchy(Node* node, const glm::mat4& fatherTransformMatrix) {

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
                if (updateNode->isActive)
                    updateNode->update();

            for (Node *child : node->children)
                updateUpdate3DNodes(child);
        }

        /** Loads the scene given its root */
        void loadScene(Node *root) {
            if (!root)
                error("This should never happen (camToChange null here)");

            info("Loading Scene");

            // Set root
            this->scene = root;

            // Load scene nodes
            this->addNodeRecursive(root);

            info("Successfully loaded scene");
        }

        /** Adds a node to the scene */
        void addNode(Node *node) {
            // Call onEnter() for the node
            node->onEnter();

            // Node specific initialization
            if (Model3D *model = dynamic_cast<Model3D*>(node)) {
                renderer.instantiate(model);
            } else if (DirectionalLight *light = dynamic_cast<DirectionalLight*>(node)) {
                renderer.createDirectionalLight(light);
            } else if (AmbientLight *light = dynamic_cast<AmbientLight*>(node)) {
                renderer.setAmbientLight(light);
            } else if (PointLight *light = dynamic_cast<PointLight*>(node)) {
                renderer.addPointLight(light);
            } else if (SpotLight *light = dynamic_cast<SpotLight*>(node)) {
                renderer.addSpotLight(light);
            } else if (Collider *coll = dynamic_cast<Collider*>(node)) {
                Physics::addCollider(coll);
            } else if (Camera *camera = dynamic_cast<Camera*>(node)) {
                if (camera->getIsMain())
                    this->setMainCamera(camera);
            } else if (AudioNode *audio = dynamic_cast<AudioNode*>(node)) {
                audio->setAudioEngine(&this->audioEngine);
            }

            log(std::format("Added Node [{}, ID: {}]", node->name, node->UUID));
        }

        /** Adds a node to the scene and also adds its descendants */
        void addNodeRecursive(Node *node)
        {
            addNode(node);
            for (Node *child : node->children)
                addNodeRecursive(child);
        }

        /** Deletes a node from the scene */
        void deleteNode(Node *node) {
            // Call onExit() for the node
            node->onExit();

            // Node specific action
            if (Model3D *model = dynamic_cast<Model3D*>(node)) {
                renderer.removeObject(model);
            } else if (dynamic_cast<DirectionalLight*>(node)) {
                renderer.unsetDirectionalLight();
            } else if (dynamic_cast<AmbientLight*>(node)) {
                renderer.unsetAmbientLight();
            } else if (PointLight *light = dynamic_cast<PointLight*>(node)) {
                renderer.removePointLight(light);
            } else if (SpotLight *light = dynamic_cast<SpotLight*>(node)) {
                renderer.removeSpotLight(light);
            } else if (Collider *coll = dynamic_cast<Collider*>(node)) {
                Physics::removeCollider(coll);
            } else if (Camera *camera = dynamic_cast<Camera*>(node)) {
                if (camera == this->mainCamera) 
                    this->mainCamera = nullptr;
            }

            log(std::format("Removed Node [{}, ID: {}]", node->name, node->UUID));

            // Free memory TODO: scenes can be reloaded?
            // NOTE: also, models being deleted now as nodes creates a lot of problems with the Renderer (after fixing leaks)
            // delete node;
        }

        /** Deletes a node from the scene and also deletes its descentants */
        void deleteNodeRecursive(Node *node)
        {
            for (Node *child : node->children)
                deleteNodeRecursive(child);

            deleteNode(node);
        }

        /** Clears the current scene */
        void clearScene() {
            // If no scene was set, no need to clear (expected during first load)
            if (!this->scene)
                return;

            info("Clearing Scene");

            // Clear scene nodes
            this->deleteNodeRecursive(this->scene);

            // Reset properties
            this->scene = nullptr;

            info("Scene successfully cleared");
        }


        /** Changes the current scene with the one in queue */
        void changeScene() {
            this->clearScene();
            this->loadScene(this->sceneToLoad);
            this->sceneToLoad = nullptr;
        }

        /** Deletes every node in the queue of nodes to be deleted */
        void deleteNodesQueuedToBeDeleted() {

            for (Node *node : this->nodesQueuedToBeDeleted) {

                // If parent isn't going to be deleted, remove the node from it
                if (!this->nodesQueuedToBeDeleted.contains(node->parent))
                    node->parent->disown(node);

                // Reparent children not queued to be deleted
                std::set<Node*> children(node->children.begin(), node->children.end());
                for (Node *child : children)
                    if (!this->nodesQueuedToBeDeleted.contains(child))
                        this->scene->adopt(child);

                this->deleteNode(node);
            }

            // Clear queue
            this->nodesQueuedToBeDeleted.clear();
        }

        /** Tries to find a camera from the scene and sets it as main. Raises an error if it can't find one.
         * @note Gives priority to cameras with the 'isMain' flag.
         * */
        void tryFindCamera() {
            warning("Main camera was not set. Trying to find one...");
            Camera *camera = tryFindCameraRecursive(this->scene);
            if (!camera)
                error("Could not find any camera");
            warning(std::format("Camera found: setting [{}] as the Main Camera", camera->UUID));
            this->setMainCamera(camera);
        }

        /** Recursive for tryFindCamera() */
        Camera* tryFindCameraRecursive(Node *node) {
            Camera *camera = nullptr;

            for (Node *child : node->children) {
                Camera *childCam = tryFindCameraRecursive(child);
                if (childCam)
                    camera = childCam;
            }

            // Set new camera if there isnt's one yet or if a 'isMain' is found
            if (Camera *cam = dynamic_cast<Camera*>(node))
                if (!camera || (!camera->getIsMain() && cam->getIsMain()))
                    camera = cam;

            return camera;
        }

        /** Reads inputs (translation, rotation), computes the deltaTime and updates the Engine globals with them */
        void updateGlobals() {
            // Reset values
            this->inputTranslation = VEC3_ZERO;
            this->inputRotation = VEC3_ZERO;
            this->deltaTime = 0.0f;
            bool fire; // Won't be used

            // Read values from Starter
            getSixAxis(this->deltaTime, this->inputTranslation, this->inputRotation, fire);

            // Capturing the rotation with no 'click' constraint from 'getSixAsix()' in Starter
            static double old_xpos = 0, old_ypos = 0;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            const double m_dx = xpos - old_xpos;
            const double m_dy = ypos - old_ypos;
            old_xpos = xpos; old_ypos = ypos;
            constexpr float MOUSE_RES = 10.0f;
            glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
            this->inputRotation.y = m_dx / MOUSE_RES;
            this->inputRotation.x = m_dy / MOUSE_RES;

        }

        /** Shuts down the Engine and closes the window */
        void shutdown() {

            info("Shutting down Engine...");

            this->clearScene();

            // Shutdown audio Engine
            ma_engine_uninit(&this->audioEngine);

            // Shutdown the window
            glfwSetWindowShouldClose(this->window, GL_TRUE);

            info("Engine successfully shut down");
        }


        /** Starts the Engine (called once before the first Engine loop) */
        void engineInit() {
            info("Starting Engine...");

            // Set default cursor mode
            log("Setting default cursor mode...");
            Engine::setCursorMode(DEFAULT_CURSOR);
            log("DONE");

            log("Starting audio engine...");
            ma_engine_init(NULL, &this->audioEngine);
            log("DONE");

            info("Engine successfully started");
            this->isFirstEngineLoop = false;
        }

        bool isFirstEngineLoop = true;

        /** Engine loop */
        void engineLoop() {

            // Initialize Engine
            if (this->isFirstEngineLoop)
                this->engineInit();

            // If a new scene is queued to be set as main, do it now
            if (this->sceneToLoad != nullptr)
                this->changeScene();

            // Delete nodes requested to be deleted
            this->deleteNodesQueuedToBeDeleted();

            // Checks
            if (!this->scene)
                error("Scene not set!");
            if (!this->mainCamera)
                this->tryFindCamera();

            // Read inputs and compute deltaTime
            this->updateGlobals();

            // Update total time
            this->time += this->deltaTime;

            // Call update() on all UpdateNodes
            this->updateUpdate3DNodes(this->scene);

            // Recompute hierarchy in case something changed
            this->recompute3DNodeHierarchy(this->scene, MAT4_I);
            this->recompute2DNodeHierarchy(this->scene, MAT4_I);

            // Physics checks
            Physics::checkCollisions();

            // Update Audio following the Main Camera
            ma_engine_listener_set_position(&this->audioEngine, AUDIO_LISTENER, this->mainCamera->getGlobalPosition().x, this->mainCamera->getGlobalPosition().y, this->mainCamera->getGlobalPosition().z);
            ma_engine_listener_set_direction(&this->audioEngine, AUDIO_LISTENER, this->mainCamera->getLookingDirection().x, this->mainCamera->getLookingDirection().y, this->mainCamera->getLookingDirection().z);
            ma_engine_listener_set_world_up(&this->audioEngine, AUDIO_LISTENER, 0.0f, 1.0f, 0.0f);

            // TODO: move txt to its node

            // updates the FPS
            static float elapsedT = 0.0f;
            static int countedFrames = 0;
            static float loadingBarSize = 1.0f;
            static int direction = 1;
            static int currentTexture = 1;

            countedFrames++;
            elapsedT += this->deltaTime;
            if(elapsedT > 1.0f) {
                float Fps = (float)countedFrames / elapsedT;

                std::ostringstream oss;
                oss << "FPS: " << Fps << "\n";

                txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});

                elapsedT = 0.0f;
                countedFrames = 0;

                // example of ui element changing size
                loadingBarSize += 1.0f * direction;
                if (loadingBarSize >= 10.0f || loadingBarSize <= 1.0f) {
                    direction = -1 * direction;
                }
                ui.renderUI(-0.95f, 0.95f, 0, UIO_LEFT, UIO_BOTTOM, loadingBarSize, 1.0f);

                // example of ui element changing texture
                ui.recreateUIDescriptorSet(1, currentTexture);
                if (currentTexture == 0)
                    currentTexture = 1;
                else
                    currentTexture = 0;
            }

        }


    protected:

        Renderer renderer;
        RenderPass RP;

        TextMaker txt; // TODO: move to node

        UIMaker ui;

        void localInit() override {

            renderer.localInit();

            RP.init(this);

            // sets the blue sky
            RP.properties[0].clearValue = {{{0.0f,0.0f,0.0f,0.0f}}};


            // Set the size of the Descriptor Set Pool
            DPSZs.uniformBlocksInPool = POOL_SIZE;
            DPSZs.texturesInPool = POOL_SIZE;
            DPSZs.setsInPool = POOL_SIZE;

            // TODO: move to node
            // initializes the textual output
            txt.init(this, windowWidth, windowHeight);

            ui.initElement(UI_ID_PAUSE_MENU_BACKGROUND, {{ProceduralTextures::generateMenuBackgroundTint(windowWidth, windowHeight)}, true, FULL_RESIZABLE});
            ui.initElement(UI_ID_BUTTON, {{"assets/textures/button.png", "assets/textures/button_hover.png", "assets/textures/button_press.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});

            ui.init(windowWidth, windowHeight, {
                {{"assets/textures/black.png"}, true},
            }, {
                {{ProceduralTextures::generateTexture(32, 32), ProceduralTextures::generateTextureWithNoise(32, 32)}},
            });

            // submits the main command buffer
            submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

            // Prepares for showing the FPS count
            txt.print(1.0f, 1.0f, "FPS:",1,"CO",false,false,true,TAL_RIGHT, TRH_RIGHT, TRV_BOTTOM, {1.0f,0.0f,0.0f,1.0f}, {0.8f,0.8f,0.0f,1.0f});
            txt.print(-1.0f, -1.0f ,  "Testo di prova", 2, "CO", false, false, false, TAL_LEFT, TRH_LEFT, TRV_TOP, {0.5f, 0.5f, 0.0f, 0.5f}, {0.5f,0.5f,0.0f,0.5f});

            ui.renderUI(-0.95f, 0.95f, 0, UIO_LEFT, UIO_BOTTOM, 1.0f, 1.0f);
            ui.renderUI(0.0f, 0.0f, UI_ID_BUTTON, UIO_CENTER, UIO_MIDDLE);
            ui.renderUI(0.0f, 0.0f, UI_ID_PAUSE_MENU_BACKGROUND, UIO_CENTER, UIO_MIDDLE);
            ui.renderUI(-1.0f, 1.0f, 1, UIO_LEFT, UIO_BOTTOM);
        }

        void pipelinesAndDescriptorSetsInit() override {
            // Update window properties in case it was resized
            RP.width = this->swapChainExtent.width;
            RP.height = this->swapChainExtent.height;

            txt.resizeScreen(this->swapChainExtent.width, this->swapChainExtent.height);
            ui.resizeScreen(this->swapChainExtent.width, this->swapChainExtent.height);

            RP.create();

            renderer.descriptorSetsInits();

            txt.pipelinesAndDescriptorSetsInit();
            ui.pipelinesAndDescriptorSetsInit();
        }

        void pipelinesAndDescriptorSetsCleanup() override {
            RP.cleanup();
            renderer.descriptorSetsCleanup();
            txt.pipelinesAndDescriptorSetsCleanup();
            ui.pipelinesAndDescriptorSetsCleanup();
        }

        void localCleanup() override {
            RP.destroy();

            renderer.localCleanup();

            txt.localCleanup();
            ui.localCleanup();
        }

        /** Populates the command buffer */
        static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
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

        /** Engine method to update the uniforms (called every frame) */
        void updateUniformBuffer(uint32_t currentImage) override {
            if (isPauseMenuOpen()) {
                glfwGetCursorPos(MainEngine->window, &cursorPosX, &cursorPosY);
                // updates the cursor in ui only if its position changed
                if (cursorPosX != oldCursorPosX || cursorPosY != oldCursorPosY) {
                    oldCursorPosX = cursorPosX;
                    oldCursorPosY = cursorPosY;
                    ui.updateMouseStatus(cursorPosX, cursorPosY);
                }
            }

            // Engine logic
            this->engineLoop();

            // Flush screen to update with node updates
            renderer.flushScreenUpdate();

            // Cleanup memory from old models


            // Renderer updates
            // Update with camera data
            renderer.updateUniformBuffer(
                    currentImage,
                    this->mainCamera->getGlobalPosition(),
                    this->mainCamera->getProjectionMatrix(),
                    this->mainCamera->getViewMatrix(), 
                    this->time);

            renderer.updateLightCulling(this->mainCamera->getGlobalPosition(), LIGHT_RENDER_DISTANCE);

            txt.updateCommandBuffer();
            ui.updateCommandBuffer();

            // Shutdown the Engine if request sfed
            if (this->shutdownRequested)
                this->shutdown();

            // Deleted models memory cleanup
            renderer.cleanDeletedModels();
        }

        /** Called when the window is created */
        void setWindowParameters() override {
            // window size, title and initial background
            windowWidth = 1080;
            windowHeight = 720;
            windowTitle = "VIDEO GAME";
            windowResizable = GLFW_TRUE;
        }

        /** Called when the window size changes */
        void onWindowResize(int w, int h) override {
            log(std::format("Window resized to {} x {}", w, h));
        }

};

#endif
