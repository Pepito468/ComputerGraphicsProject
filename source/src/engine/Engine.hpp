// ENGINE

#include <sstream>
#include <json.hpp>

#include "common.h"

#include "Renderer.hpp"
#include "Node.hpp"
#include "Node3D.hpp"

class Engine : public BaseProject {
    // NOTE: new
    public:
    Engine() : renderer(this, &RP, [this](){submitCommandBuffer("main", 0, populateCommandBufferAccess, this);}) {

    }

    protected:
    // Here you list all the Vulkan objects you need:

    Renderer renderer;

    RenderPass RP;

    // to provide textual feedback
    TextMaker txt;
    
    // Other application parameters
    float Ar;    // Aspect ratio

    glm::mat4 ViewPrj;
    glm::mat4 View;
    
    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Skeleton: place the name of your app here";
        windowResizable = GLFW_TRUE;
        
        // Initial aspect ratio
        Ar = 4.0f / 3.0f;
    }
    
    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float)w / (float)h;
        // Update Render Pass
        RP.width = w;
        RP.height = h;
        
        // updates the textual output
        txt.resizeScreen(w, h);
    }
    
    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines

    //TODO

    LambertMaterial mat3 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}};

    //LambertMaterial mat2 = {glm::vec3(0.0f, 0.0f, 1.0f), {1.0f,1.0f,1.0f,50.0f}};

    //ToonMaterial mat1 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
    ToonMaterial mat2 = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};

    LambertTexMaterial mat1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};

    //LambertTexMaterial mat2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "VChecker.png"};

    std::string modelPath = "SuzanneUV.obj";

    Model3D m = {modelPath , {1.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat1};
    Model3D m1 = {modelPath, {1.0f, 1.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat2};
    Model3D m2 = {modelPath, {1.0f, 2.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat1};
    Model3D m3 = {modelPath, {1.0f, 3.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat2};
    Model3D m4 = {modelPath, {1.0f, 4.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat1};

    Model3D m5 = {modelPath, {-1.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat2};
    Model3D m6 = {modelPath, {-1.0f, 1.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat1};
    Model3D m7 = {modelPath, {-1.0f, 2.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat2};
    Model3D m8 = {modelPath, {-1.0f, 3.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat1};
    Model3D m9 = {modelPath, {-1.0f, 4.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(0.5f), &mat2};

    Model3D plane = {"Plane.gltf", {0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(10.0f), &mat3};


    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]

        renderer.loadSceneFromJSON();

        renderer.setAmbientLight(glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(0.2f, 0.15f, 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
        renderer.createDirectionalLight(0.01f, glm::vec3(1.0f, 0.95f, 0.8f), glm::normalize(glm::vec3(0.4f, 0.8f, 0.4f)));

        //renderer.addPointLight(glm::vec3(-1.0f, 2.0f, -5.0f), 0.1f, glm::vec3(1.0f, 1.0f, 1.0f), 5.0f, 2.0f);
        //renderer.addPointLight(glm::vec3(-3.0f, 2.0f, 4.0f), 0.1f, glm::vec3(0.0f, 0.0f, 1.0f), 5.0f, 2.0f);
        //renderer.addPointLight(glm::vec3(2.0f, 2.0f, -1.0f), 0.1f, glm::vec3(1.0f, 0.0f, 0.0f), 5.0f, 2.0f);
        //renderer.addPointLight(glm::vec3(4.0f, 4.0f, -3.0f), 0.1f, glm::vec3(0.0f, 1.0f, 0.0f), 5.0f, 2.0f);

        //renderer.addSpotLight(glm::vec3(0.0f, 7.0f, 0.0f), 0.1f, glm::vec3(1.0f, 1.0f, 1.0f), 5.0f, 10.0f, glm::normalize(glm::vec3(0.0f, 1.0f,0.0f)));
        //renderer.addSpotLight(glm::vec3(3.0f, 7.0f, 0.0f), 0.1f, glm::vec3(1.0f, 1.0f, 0.0f), 5.0f, 10.0f, glm::normalize(glm::vec3(0.0f, 1.0f,0.0f)));
        //renderer.addSpotLight(glm::vec3(-3.0f, 7.0f, 0.0f), 0.1f, glm::vec3(1.0f, 0.0f, 1.0f), 5.0f, 10.0f, glm::normalize(glm::vec3(0.0f, 1.0f,0.0f)));
        //renderer.addSpotLight(glm::vec3(-3.0f, 7.0f, 4.0f), 0.1f, glm::vec3(0.0f, 0.0f, 1.0f), 5.0f, 10.0f, glm::normalize(glm::vec3(0.0f, 1.0f,0.0f)));

        //std::cout << "TEST\n";

        //renderer.preLoadModel(&plane);

        /*
        renderer.instantiate(&m);
        renderer.instantiate(&m1);
        renderer.instantiate(&m2);
        renderer.instantiate(&m3);
        renderer.instantiate(&m4);
        renderer.instantiate(&m5);
        renderer.instantiate(&m6);
        renderer.instantiate(&m7);
        renderer.instantiate(&m8);
        renderer.instantiate(&m9);
        */

        renderer.localInit();

        // initializes the render passes
        RP.init(this);
        // sets the blue sky
        RP.properties[0].clearValue = {0.0f,0.0f,0.0f,0.0f};


        // sets the size of the Descriptor Set Pool (it MUST be done before loading the scene)
        DPSZs.uniformBlocksInPool = 200;
        DPSZs.texturesInPool = 200;
        DPSZs.setsInPool = 200;

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

    glm::vec3 CamPos = {0.0f, 1.0f, 4.0f};
    float Pitch = 0.0f, Yaw = 0.0f;
    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    int i = 0, j = 0;
    bool test = true, test_ = true, test__ = true;
    void updateUniformBuffer(uint32_t currentImage) {
        static bool debounce = false;
        static int curDebounce = 0;

        // handle the ESC key to exit the app
        if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        /*
        if (glfwGetKey(window, GLFW_KEY_Q)) {
            if (test) {
                renderer.removeObject(0);
                test = false;

                submitCommandBuffer("main", 0, populateCommandBufferAccess, this);
            }
        } else {
            test = true;
        }*/
int n;
        if (glfwGetKey(window, GLFW_KEY_Z)) {
            if (test__) {
                if (j%2 == 0 ) {
                    renderer.instantiate("Statue.gltf" , {1.0f, j, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(1.0f), &mat2);
                }
                else {
                    renderer.instantiate("Statue.gltf" , {-1.0f, j, 0.0f},{0.0f, 0.0f, 0.0f},glm::vec3(1.0f), &mat2);
                }

                test__ = false;

                j++;
            }
        } else {
            test__ = true;
        }

        if (glfwGetKey(window, GLFW_KEY_Q)) {
            if (test) {
                renderer.setObjectVisibility(i, false);
                i++;
                test = false;
            }
        } else {
            test = true;
        }

        if (glfwGetKey(window, GLFW_KEY_E)) {
            if (test_) {
                i--;
                if (i < 0)
                    i = 0;

                renderer.setObjectVisibility(i, true);

                test_ = false;
            }
        } else {
            test_ = true;
        }

        glm::mat4 View, Projection;
        // Camera FOV-y, Near Plane and Far Plane
        const float FOVy = glm::radians(45.0f);
        const float nearPlane = 0.1f;
        const float farPlane = 100.f;

        float deltaT;
        glm::vec3 m = {0,0,0}, r = {0,0,0};
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        Projection = glm::perspective(FOVy, Ar, nearPlane, farPlane);
        Projection[1][1] *= -1;

        // update the camera position and direction with the inputs
        CamPos += m * 2.0f * deltaT;
        Pitch -= r.x * deltaT;
        Yaw   -= r.y * deltaT;

        View = glm::rotate   (glm::mat4(1), -Pitch, glm::vec3(1,0,0)) *
               glm::rotate   (glm::mat4(1), -Yaw,   glm::vec3(0,1,0)) *
               glm::translate(glm::mat4(1), -CamPos);

        renderer.updateUniformBuffer(currentImage, CamPos, Projection, View);

        renderer.updateLightCulling(CamPos, 10.0f);

        
        // updates the FPS
        static float elapsedT = 0.0f;
        static int countedFrames = 0;
        
        countedFrames++;
        elapsedT += deltaT;
        if(elapsedT > 1.0f) {
            float Fps = (float)countedFrames / elapsedT;
            
            std::ostringstream oss;
            oss << "FPS: " << Fps << "\n";

            txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true,TAL_RIGHT,TRH_RIGHT,TRV_BOTTOM,{1.0f,0.0f,0.0f,1.0f},{0.8f,0.8f,0.0f,1.0f});
            
            elapsedT = 0.0f;
            countedFrames = 0;
        }
        
        txt.updateCommandBuffer();
    }
    
    float GameLogic() {
        // Camera FOV-y, Near Plane and Far Plane
        const float FOVy = glm::radians(45.0f);
        const float nearPlane = 0.1f;
        const float farPlane = 100.f;

        // Integration with the timers and the controllers
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        // Projection
        glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
        Prj[1][1] *= -1;

        // View
        View = glm::lookAt(glm::vec3(0.0f, 1.0f, 5.0f), // Pos
                           glm::vec3(0.0f),                // Target
                           glm::vec3(0.0f, 1.0f, 0.0f));

        // View-Projection
        ViewPrj = Prj * View;

        return deltaT;
    }
};
