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
#include "common.h"
#include "Physics.hpp"
#include "UIMaker.hpp"

#define POOL_SIZE 1000
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
        float time = 0.0f;
        // Translation input
        glm::vec3 inputTranslation = VEC3_ZERO;
        // Rotation input
        glm::vec3 inputRotation = VEC3_ZERO;
        // Avoid multiple triggers of functions when a button is being pressed
        bool debounce = false;
        // Key target of the debounce
        int curDebounce = 0;

        /** Root of the current scene */
        Node *scene = nullptr;

        /** Main camera (camera that is being used to render) */
        Camera *mainCamera = nullptr;

        /** This holds the value of a scene that has been requested to be loaded by someone. It will be loaded at the next Engine loop */
        Node *sceneToLoad = nullptr;

        std::set<Node*> nodesQueuedToBeDeleted;

        std::map<std::string, Node*> scenes;



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

        static bool getDebounce() {
            return MainEngine->debounce;
        }

        static void setDebounce(bool _debounce) {
            MainEngine->debounce = _debounce;
        }

        static int getCurDebounce() {
            return MainEngine->curDebounce;
        }

        static void setCurDebounce(int glfwKey) {
            MainEngine->curDebounce = glfwKey;
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
                warning("Requesting a null scene does nothing");
                return;
            }

            if (newRoot == MainEngine->scene) {
                warning("Trying to load the current scene again");
                return;
            }

            MainEngine->sceneToLoad = newRoot;
            log(std::format("Scene change request accepted [to {}]", newRoot->name));
        }

        /** Maps a name to a scene (for convenience) */
        static void mapScene(std::string sceneName, Node* sceneRoot) {
            MainEngine->scenes[sceneName] = sceneRoot;
        }

        /** Returns the scene mapped with its name (or nullptr if no scene was mapped with that name) */
        static Node* getSceneFromNameMap(std::string sceneName) {
            return MainEngine->scenes[sceneName];
        }

        /** Adds a child to the given father */
        static void addChild(Node *father, Node *child) {
            if (!father || !child)
                error("Assigning NULL Nodes in addChild");

            father->adopt(child);
            MainEngine->addNode(child);
        }

        /** Removes a node from the scene and frees it */
        static void freeNode(Node *node) {
            if (!node)
                error("Attempting to free a NULL Node");

            if (node == MainEngine->scene)
                error("Can't free the root, request a scene change instead");

            // Disown
            node->parent->disown(node);

            // Transfer children
            std::set<Node*> children;
            for (Node *child : node->children)
                children.insert(child);
            for (Node *child : children)
                MainEngine->scene->adopt(child);

            MainEngine->removeNode(node);
        }

        /** Queue a node for deletion.
         * NOTE: it will be actually deleted in the next Engine loop */
        static void requestNodeDeletion(Node *node) {
            if (!node)
                error("Deleting NULL node");

            MainEngine->nodesQueuedToBeDeleted.insert(node);
            log(std::format("Node deletion request accepted [{}]", node->name));
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
                if (updateNode->isActive)
                    updateNode->update();

            for (Node *child : node->children)
                updateUpdate3DNodes(child);
        }

        /** Sets the given camera as main camera */
        void setMainCamera(Camera *camera) {
            if(!camera)
                error("Setting invalid camera!");

            if (this->mainCamera) {
                warning(std::format("Trying to set [{}] as Main Camera, but [{}] is already the Main Camera. Ignoring...", camera->UUID, this->mainCamera->UUID));
                return;
            }

            this->mainCamera = camera;
        }

        /** Loads the scene given its root */
        void loadScene(Node *root) {
            if (!root)
                error("This should never happen (camToChange null here)");

            info("Loading Scene");

            // Set root
            this->scene = root;

            // Load scene nodes
            this->loadSceneRecursive(root);

            // TODO: physics handling of new colliders being added
            Physics::loadScene(this->scene);


            log(std::format("Loaded {} rendering objects", renderer.getTotalObjectCount()));
            info("Succesfully loaded scene");
        }

        /** Loads every scene node */
        void loadSceneRecursive(Node *node) {
            addNode(node);
            for (Node *child : node->children)
                loadSceneRecursive(child);
        }

        /** Adds a node */
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
            } else if (Camera *camera = dynamic_cast<Camera*>(node)) {
                if (camera->getIsMain())
                    this->setMainCamera(camera);
            }

            log(std::format("Added Node [{}, ID: {}]", node->name, node->UUID));
        }

        /** Clears the current scene */
        void clearScene() {
            // If no scene was set, no need to clear (expected during first load)
            if (!this->scene)
                return;

            info("Clearing Scene");

            size_t poolSize = renderer.getTotalObjectCount();
            // Clear scene nodes
            this->clearSceneRecursive(this->scene);

            // Reset properties
            this->scene = nullptr;
            this->mainCamera = nullptr;

            // TODO: physics handling of colliders being removed
            // Physics::deleteScene(MainEngine->scene);

            log(std::format("Cleared {} rendering objects out of {}", poolSize - renderer.getTotalObjectCount(), poolSize));
            info("Scene successfully cleared");

        }

        /** Clears every scene node */
        void clearSceneRecursive(Node *node) {

            for (Node *child : node->children)
                clearSceneRecursive(child);

            removeNode(node);
        }

        /** Removes a node */
        void removeNode(Node *node) {
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
            } else if (Camera *camera = dynamic_cast<Camera*>(node)) {
                if (camera->getIsMain()) // TODO: may add back the setMainCamera()
                    this->setMainCamera(nullptr);
            }

            log(std::format("Removed Node [{}, ID: {}]", node->name, node->UUID));

            // Free memory ( TODO: should the Engine free the node?)
            delete node;

        }

        /** Changes the current scene with the one in queue */
        void changeScene() {
            this->clearScene();
            this->loadScene(this->sceneToLoad);
            this->sceneToLoad = nullptr;
        }

        /** Starts the Engine (called once before the first Engine loop) */
        void engineInit() {
            info("Starting Engine...");

            // Set default cursor mode
            Engine::setCursorMode(DEFAULT_CURSOR);

            info("Engine successfully started");
        }

        bool isFirstEngineLoop = true;

        /** Engine loop */
        void engineLoop() {

            // Initialize Engine
            if (this->isFirstEngineLoop) {
                this->engineInit();
                this->isFirstEngineLoop = false;
            }

            // If a new scene is queued to be set as main, do it now
            if (this->sceneToLoad != nullptr)
                this->changeScene();

            // Delete nodes requested to be deleted
            for (Node *node : this->nodesQueuedToBeDeleted)
                freeNode(node);
            this->nodesQueuedToBeDeleted.clear();

            // Flush screen
            this->renderer.flushScreenUpdate();

            // Checks
            if (!this->scene)
                error("Scene not set!");
            if (!this->mainCamera)
                error("Main camera not set");

            // Read inputs and compute deltaTime
            this->computeGlobals();

            // Update total time
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

                loadingBarSize += 1.0f * direction;
                if (loadingBarSize >= 10.0f || loadingBarSize <= 1.0f) {
                    direction = -1 * direction;
                }
                ui.renderUI(-0.95f, 0.95f, 0, UIO_LEFT, UIO_BOTTOM, loadingBarSize, 1.0f);

                ui.recreateUIDescriptorSet(1, currentTexture);
                if (currentTexture == 0)
                    currentTexture = 1;
                else
                    currentTexture = 0;
            }

        }

        /** Reads inputs (translation, rotation), computes the deltaTime and updates the Engine globals with them */
        void computeGlobals() {
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
            double m_dx = xpos - old_xpos;
            double m_dy = ypos - old_ypos;
            old_xpos = xpos; old_ypos = ypos;
            const float MOUSE_RES = 10.0f;
            glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
            this->inputRotation.y = m_dx / MOUSE_RES;
            this->inputRotation.x = m_dy / MOUSE_RES;

        }


    protected:

        Renderer renderer;
        RenderPass RP;

        TextMaker txt; // TODO: move to node

        UIMaker ui;

        void localInit() {

            // renderer.loadSceneFromJSON();
            renderer.localInit();

            // initializes the render passes
            RP.init(this);

            // sets the blue sky
            RP.properties[0].clearValue = {{{0.0f,0.0f,0.0f,0.0f}}};


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

        void pipelinesAndDescriptorSetsInit() {
            RP.create();

            renderer.descriptorSetsInits();

            txt.pipelinesAndDescriptorSetsInit();
            ui.pipelinesAndDescriptorSetsInit();
        }

        void pipelinesAndDescriptorSetsCleanup() {
            RP.cleanup();
            renderer.descriptorSetsCleanup();
            txt.pipelinesAndDescriptorSetsCleanup();
            ui.pipelinesAndDescriptorSetsCleanup();
        }

        void localCleanup() {
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
        void updateUniformBuffer(uint32_t currentImage) {
            bool isCursorAvailable = MainEngine->getCursorMode() == GLFW_CURSOR_NORMAL;
            if (isCursorAvailable) {
                double x, y;
                glfwGetCursorPos(MainEngine->window, &x, &y);
                ui.setMousePosition(x, y);
            }

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
            ui.updateCommandBuffer(isCursorAvailable);
        }

        /** Called when the window is created */
        void setWindowParameters() {
            // window size, titile and initial background
            windowWidth = 1080;
            windowHeight = 720;
            windowTitle = "VIDEOGAME";
            windowResizable = GLFW_TRUE;
        }

        /** Called when the window size changes */
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
