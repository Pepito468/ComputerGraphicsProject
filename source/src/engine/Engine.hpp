// ENGINE
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

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

#include "Text2D.hpp"
#include "UIMaker.hpp"

#include "audio/AudioNode.hpp"

#include "common.h"

#define POOL_SIZE 10000
#define LIGHT_RENDER_DISTANCE 20.0f
#define DEFAULT_CURSOR GLFW_CURSOR_NORMAL
#define AUDIO_LISTENER 0
#define TEXT2D_SAFE_INDEX -2
#define TEXT2D_UPDATE_INTERVAL 1.0f
#define MAX_MOUSE_RES 10.0f

class Engine : public BaseProject {

    public:
        /** Static reference to the only Engine */
        inline static Engine *MainEngine = nullptr;


    private:

        // Time elapsed from last frame
        float deltaTime = 0.0f;
        // Time elapsed since the start
        float currentTime = 0.0f;
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

        std::map<std::string, std::any> globalVariables;

        ma_engine audioEngine;

        /// Stores the cursor position
        double cursorPosX = 0, cursorPosY = 0, oldCursorPosX = 0, oldCursorPosY = 0;

        bool shutdownRequested = false;

        bool leftMouseButtonDown = false;

        bool canPause = false;

        // Text
        TextMaker textEngine;

        float globalVolume = 1.0f;

        float mouseRes = MAX_MOUSE_RES;

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

        static float getCurrentTime() {
            return MainEngine->currentTime;
        }

        /** Returns true if the given key is being pressed */
        static bool isKeyBeingPressed(const int key, const bool onlyOnPress = false) {
            // glfwGetKey doesn't work with mouse buttons
            if (key == GLFW_MOUSE_BUTTON_LEFT) {
                if (glfwGetMouseButton(MainEngine->window, key) == GLFW_PRESS && !MainEngine->leftMouseButtonDown) {
                    MainEngine->leftMouseButtonDown = true;
                    return true;
                } else if (glfwGetMouseButton(MainEngine->window, key) == GLFW_RELEASE) {
                    MainEngine->leftMouseButtonDown = false;
                    return false;
                }
            } else if (glfwGetKey(MainEngine->window, key)) {
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

        /** Returns the scene's root */
        static Node* getSceneRoot() {
            return MainEngine->scene;
        }

        /** Sets the master volume */
        static void setMasterVolume(const float volume) {
            ma_engine_set_volume(&MainEngine->audioEngine, volume);
            MainEngine->globalVolume = volume;
        }

        /** Sets the mouse res */
        static void setMouseRes(const float mouseRes) {
            MainEngine->mouseRes = mouseRes;
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

        /** Maps a name to a global variable. */
        static void setGlobalVariable(const std::string& name, std::any value) {
            MainEngine->globalVariables[name] = value;
        }

        /** Returns the global variable mapped with its name.
         * @note returns nullptr if no variable is mapped to the given name
         */
        static auto getGlobalVariable(const std::string& name) {
            return MainEngine->globalVariables[name];
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

        /** Communicates to the ui that something was clicked in the menu, and handles calling of engine functions */
        static void handleMenuMouse(bool click, double mouseX, double mouseY) {
            for (auto data : MainEngine->ui.updateMouseStatus(mouseX, mouseY, click, MainEngine->leftMouseButtonDown)) {
                switch (data.id) {
                case UI_ID_BUTTON_RESUME:
                    Engine::setCursorMode(GLFW_CURSOR_DISABLED);
                    MainEngine->ui.togglePauseMenu();
                    break;
                case UI_ID_BUTTON_QUIT:
                    Engine::MainEngine->requestEngineShutdown();
                    break;
                case UI_ID_BUTTON_SCENE1:
                    Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Scene1")));
                    break;
                case UI_ID_BUTTON_SCENE2:
                    Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Scene2")));
                    break;
                case UI_ID_SLIDER_VOLUME:
                    setMasterVolume(data.data);
                    break;
                case UI_ID_SLIDER_SENSITIVITY:
                    if (data.data <= 0.01f)
                        data.data = 0.01f;

                    setMouseRes(MAX_MOUSE_RES/data.data);
                    break;
                case UI_ID_BUTTON_START:
                    MainEngine->ui.toggleVisibility(UI_ID_BUTTON_START);
                    MainEngine->ui.toggleVisibility(UI_ID_TITLE);
                    Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Forest")));
                    MainEngine->ui.renderPauseMenu();
                    Engine::setCursorMode(GLFW_CURSOR_DISABLED);
                    MainEngine->ui.togglePauseMenu();
                    MainEngine->canPause = true;
                    break;
                default:
                    break;
                }
            }
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

        /** Renders all Text2Ds in the tree */
        void renderText2D(Node *node) {
            if (Text2D *txt = dynamic_cast<Text2D*>(node))
                this->textEngine.print(txt->getGlobalPosition().x,
                        txt->getGlobalPosition().y, txt->text,
                        txt->textID,
                        txt->fontFace,
                        txt->isItalic,
                        txt->isBold,
                        txt->isSmall,
                        txt->alignment,
                        txt->regH,
                        txt->regV,
                        txt->color,
                        txt->stroke,
                        txt->shadow,
                        txt->getGlobalScale().x,
                        txt->getGlobalScale().y);

            for (Node *child : node->children)
                renderText2D(child);
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
            } else if (AudioNode *audio = dynamic_cast<AudioNode*>(node)) {
                audio->stopSound();
            }

            log(std::format("Removed Node [{}, ID: {}]", node->name, node->UUID));

            // NOTE: I decided to relegate memory cleanup to the user, since node
            // deletion causes a lot of issues with the Model cleanup.
            // I might try smart pointers later
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
            glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
            this->inputRotation.y = m_dx / this->mouseRes;
            this->inputRotation.x = m_dy / this->mouseRes;

        }

        /** Shuts down the Engine and closes the window */
        void shutdown() {

            info("Shutting down Engine...");

            // Clear scene
            this->clearScene();

            // Shutdown textEngine
            this->textEngine.removeAllText();

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
            this->currentTime += this->deltaTime;

            // Call update() on all UpdateNodes
            this->updateUpdate3DNodes(this->scene);

            // Check for escape
            if (isKeyBeingPressed(GLFW_KEY_BACKSPACE) || isKeyBeingPressed(GLFW_KEY_DELETE)) {
                requestEngineShutdown();
            }

            // Pause game
            if (canPause && isKeyBeingPressed(GLFW_KEY_ESCAPE, true)) {
                if (Engine::isPauseMenuOpen()) {
                    Engine::setCursorMode(GLFW_CURSOR_DISABLED);
                } else {
                    Engine::setCursorMode(GLFW_CURSOR_NORMAL);
                }
                
                MainEngine->ui.togglePauseMenu();
            }

            // Recompute hierarchy in case something changed
            this->recompute3DNodeHierarchy(this->scene, MAT4_I);
            this->recompute2DNodeHierarchy(this->scene, MAT4_I);

            // Update Text2Ds (NOTE: updating it every frame crashes, so update it every some time)
            static float elapsedTimeForText = TEXT2D_UPDATE_INTERVAL;
            elapsedTimeForText += this->deltaTime;
            if (elapsedTimeForText >= TEXT2D_UPDATE_INTERVAL) {
                textEngine.removeAllText();
                // If nothing is printed, TextMaker crashes, so print something regardless
                textEngine.print(0.0f, 0.0f, " ", TEXT2D_SAFE_INDEX, "SS");
                this->renderText2D(this->scene);
                elapsedTimeForText = 0.0f;
            }

            // Physics checks
            Physics::checkCollisions();

            // Update Audio following the Main Camera
            ma_engine_listener_set_position(&this->audioEngine, AUDIO_LISTENER, this->mainCamera->getGlobalPosition().x, this->mainCamera->getGlobalPosition().y, this->mainCamera->getGlobalPosition().z);
            ma_engine_listener_set_direction(&this->audioEngine, AUDIO_LISTENER, this->mainCamera->getLookingDirection().x, this->mainCamera->getLookingDirection().y, this->mainCamera->getLookingDirection().z);
            ma_engine_listener_set_world_up(&this->audioEngine, AUDIO_LISTENER, 0.0f, 1.0f, 0.0f);
        }


    protected:
        Renderer renderer;
        RenderPass RP;

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

            textEngine.init(this, windowWidth, windowHeight);

            // UIElements for the main menu
            ui.initElement(UI_ID_MENU_BACKGROUND, {{ProceduralTextures::generateMenuBackgroundTint(windowWidth, windowHeight)}, true, FULL_RESIZABLE});
            // ui.initElement(UI_ID_MENU_BACKGROUND, {{"assets/textures/ui/background.png"}, true, FULL_RESIZABLE});
            ui.initElement(UI_ID_TITLE, {{ProceduralTextures::generateTexture(100, 100)}, true, KEEP_ASPECT_RATIO}); //TODO add title texture 
            ui.initElement(UI_ID_BUTTON_START, {{"assets/textures/ui/start_button.png", "assets/textures/ui/start_button_hover.png", "assets/textures/ui/start_button_click.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});
            ui.initElement(UI_ID_BUTTON_QUIT, {{"assets/textures/ui/quit_button.png", "assets/textures/ui/quit_button_hover.png", "assets/textures/ui/quit_button_click.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});

            // UIElements for the pause menu
            ui.initElement(UI_ID_BUTTON_RESUME, {{"assets/textures/ui/resume_button.png", "assets/textures/ui/resume_button_hover.png", "assets/textures/ui/resume_button_click.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});
            ui.initElement(UI_ID_BUTTON_SCENE1, {{"assets/textures/ui/scene1_button.png", "assets/textures/ui/scene1_button_hover.png", "assets/textures/ui/scene1_button_click.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});
            ui.initElement(UI_ID_BUTTON_SCENE2, {{"assets/textures/ui/scene2_button.png", "assets/textures/ui/scene2_button_hover.png", "assets/textures/ui/scene2_button_click.png"}, false, KEEP_ASPECT_RATIO, UI_BUTTON});
            ui.initElement(UI_ID_SLIDER_VOLUME, {{"assets/textures/ui/slider_juice.png"}, false, KEEP_ASPECT_RATIO, UI_SLIDER});
            ui.initElement(UI_ID_SLIDER_VOLUME_BACKGROUND, {{"assets/textures/ui/slider_border_appeasement.png"}, true, KEEP_ASPECT_RATIO, UI_NORMAL});
            ui.initElement(UI_ID_SLIDER_VOLUME_PLAQUE, {{"assets/textures/ui/volume_high.png", "assets/textures/ui/volume.png", "assets/textures/ui/volume_low.png"}, true, KEEP_ASPECT_RATIO, UI_NORMAL});
            ui.initElement(UI_ID_SLIDER_SENSITIVITY, {{"assets/textures/ui/slider_juice_alt.png"}, false, KEEP_ASPECT_RATIO, UI_SLIDER});
            ui.initElement(UI_ID_SLIDER_SENSITIVITY_BACKGROUND, {{"assets/textures/ui/slider_border_appeasement.png"}, true, KEEP_ASPECT_RATIO, UI_NORMAL});
            ui.initElement(UI_ID_SLIDER_SENSITIVITY_PLAQUE, {{"assets/textures/ui/sensitivity.png"}, true, KEEP_ASPECT_RATIO, UI_NORMAL});
            ui.initElement(UI_ID_COMMANDS, {{"assets/textures/ui/commands.png"}, true, KEEP_ASPECT_RATIO, UI_NORMAL});

            ui.init(windowWidth, windowHeight);

            // submits the main command buffer
            submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

            ui.renderMainMenu();
        }


        void pipelinesAndDescriptorSetsInit() override {
            // Update window properties in case it was resized
            RP.width = this->swapChainExtent.width;
            RP.height = this->swapChainExtent.height;

            textEngine.resizeScreen(this->swapChainExtent.width, this->swapChainExtent.height);
            ui.resizeScreen(this->swapChainExtent.width, this->swapChainExtent.height);

            RP.create();

            renderer.descriptorSetsInits();

            textEngine.pipelinesAndDescriptorSetsInit();
            ui.pipelinesAndDescriptorSetsInit();
        }

        void pipelinesAndDescriptorSetsCleanup() override {
            RP.cleanup();
            renderer.descriptorSetsCleanup();
            textEngine.pipelinesAndDescriptorSetsCleanup();
            ui.pipelinesAndDescriptorSetsCleanup();
        }

        void localCleanup() override {
            RP.destroy();

            renderer.localCleanup();

            textEngine.localCleanup();
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
            // checks for mouse inputs in the pause menu
            if (isPauseMenuOpen()) {
                bool click = false, oldLeftMouseButtonDown = MainEngine->leftMouseButtonDown;
                if (Engine::isKeyBeingPressed(GLFW_MOUSE_BUTTON_LEFT, true)) {
                    click = true;
                }

                glfwGetCursorPos(MainEngine->window, &cursorPosX, &cursorPosY);
                // updates the cursor in ui only if its position changed
                if (click || cursorPosX != oldCursorPosX || cursorPosY != oldCursorPosY || oldLeftMouseButtonDown != MainEngine->leftMouseButtonDown) {
                    oldCursorPosX = cursorPosX;
                    oldCursorPosY = cursorPosY;
                    handleMenuMouse(click, cursorPosX, cursorPosY);
                }
            }

            // Engine logic
            this->engineLoop();

            // Flush screen to update with node updates
            renderer.flushScreenUpdate();

            // Update with camera data
            renderer.updateUniformBuffer(
                    currentImage,
                    this->mainCamera->getGlobalPosition(),
                    this->mainCamera->getProjectionMatrix(),
                    this->mainCamera->getViewMatrix(), 
                    this->currentTime,
                    deltaTime);

            renderer.updateLightCulling(this->mainCamera->getGlobalPosition(), LIGHT_RENDER_DISTANCE);

            textEngine.updateCommandBuffer();
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
