#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "common.h"
#include "AmbientLight.hpp"
#include "SpotLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "Model3D.hpp"


struct ShadowMapUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

struct SceneDepthUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

struct SceneColorUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

struct SkyboxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

/** This class simplifies 3D object rendering
 *
 * First you need to configure a render object inside the Engine class,then this class will offer:
 * <li>loadSceneFromJSON()</li>
 * <li>instantiate() - show on screen a new object [runtime only]</li>
 * <li>preLoadModel() - show on screen a new object [compile time only]</li>
 * <li>getObject()</li>
 * <li>removeObject()</li>
 */
class Renderer {

    ///This class handle a single pipeline. Each pipeline is associate with a unique shader
    class PipelineRenderer {
        //Shader infos
        ShaderType shaderType;
        std::string vertShader;
        std::string fragShader;

        //Collection of models sharing the same shader
        std::vector <Model3D*> pool;

        //Vulkan variables
        DescriptorSetLayout* localLayout;
        Pipeline pipeline;

        public:

        PipelineRenderer(DescriptorSetLayout* localLayout, ShaderType type = ShaderType::LAMBERT_BLINN) : shaderType(type) {
            vertShader = "shaders/" + getShaderVertName(shaderType) + ".vert.spv";
            fragShader = "shaders/" + getShaderFragName(shaderType) + ".frag.spv";

            this->localLayout = localLayout;
        }
        ~PipelineRenderer() = default;

        ///Must be called inside Renderer.localInit()
        void localInit(BaseProject* bp, DescriptorSetLayout& globalLayout, DescriptorSetLayout& offScreenLayout, VertexDescriptor& vertexDescriptor) {
            pipeline.init(bp, &vertexDescriptor, vertShader, fragShader, {&globalLayout, localLayout, &offScreenLayout});

            if (IsLateDraw(shaderType))
                pipeline.setTransparency(true);
        }

        ///Must be called inside Renderer.descriptorSetsInits()
        void descriptorSetsInits(BaseProject* bp, RenderPass* rp, RenderPass* RPoffScreen, RenderPass* RPsceneDepth, RenderPass* RPsceneColor) {
            pipeline.create(rp);

            for (auto& p : pool) {
                modelDescriptorSetInit(bp, p, RPoffScreen, RPsceneDepth, RPsceneColor);
            }
        }

        //TODO: non so se mi piace questa soluzione
        void modelDescriptorSetInit(BaseProject* bp, Model3D* model, RenderPass* RPoffScreen, RenderPass* RPsceneDepth, RenderPass* RPsceneColor) {
            model->descriptorSetInit(bp, localLayout, RPoffScreen, RPsceneDepth, RPsceneColor);
        }

        ///Must be called inside Renderer.descriptorSetsCleanup()
        void descriptorSetsCleanup() {
            pipeline.cleanup();
            for (auto& p : pool) {
                p->descriptorSetCleanup();
            }
        }

        ///Must be called inside Renderer.localCleanup()
        void localCleanup() {
            pipeline.destroy();
        }

        ///Must be called inside Renderer.populateCommandBuffer()
        void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, DescriptorSet& global, DescriptorSet& offScreen) {
            pipeline.bind(commandBuffer);
            global.bind(commandBuffer, pipeline, 0, currentImage);
            offScreen.bind(commandBuffer, pipeline, 2, currentImage);

            for (auto& p : pool) {
                    p->populateCommandBuffer(commandBuffer, currentImage, pipeline);
            }
        }

        ///Must be called inside Renderer.updateUniformBuffer()
        void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {

            for (auto& p : pool) {
                p->updateUniformBuffer(currentImage, Projection, View);
            }
        }

        ///Add a 3D model to the pool
        void addModel3D(Model3D* model) {
            pool.push_back(model);
        }

        ///Remove the specified 3D model from the pool
        bool removeModel3D(Model3D* model) {
            auto it = std::find(pool.begin(), pool.end(), model);

            if (it != pool.end()) {
                pool.erase(it);
            }

            return pool.empty();
        }

        size_t poolSize() const { return pool.size(); }
    };

    // Variable to update screen
    bool screenDirty = false;

    //Vulkan variables
    DescriptorSetLayout globalLayout, localLayout, skyboxLayout;
    DescriptorSet global;
    VertexDescriptor vertexDescriptor, VDskybox;

    //Variable taken from engine
    BaseProject* bp;
    RenderPass* rp;

    //Collections to optimize memory usage
    std::unordered_map<ShaderType, std::unique_ptr<PipelineRenderer>> pipelinesMap;
    std::unordered_map <std::string, Model> modelAssets;
    std::unordered_map <std::string, Texture> albedoTexAssets;
    std::unordered_map <std::string, Texture> armTexAssets;
    std::unordered_map <std::string, Texture> normalTexAssets;
    std::unordered_map <std::string, std::unique_ptr<Material>> materialAssets;
    std::vector<Model3D*> sceneObjects; //stores all the models assigned to Renderer

    DirectionalLight *directionalLight = nullptr;
    std::vector<PointLight*> pointlights;
    std::vector<SpotLight*> spotlights;
    AmbientLight *ambientLight = nullptr;

    //Shadow map
    DescriptorSetLayout offScreenLayout;
    DescriptorSet offScreen, skybox;
    RenderPass RPoffScreen;
    Pipeline PoffScreen, Pskybox;

    //Scene depth
    DescriptorSetLayout sceneDepthLayout;
    DescriptorSet sceneDepth;
    RenderPass RPsceneDepth;
    Pipeline PsceneDepth;

    //Scene color
    DescriptorSetLayout sceneColorLayout;
    DescriptorSet sceneColor;
    RenderPass RPsceneColor;
    Pipeline PsceneColor;

    Texture TenvMap;   // environment cubemap
    Model SkyboxCube;

    //Lambda function taken from Engine
    std::function<void()> screenUpdate;

    public:

    /**Call this inside Engine constructor, passing:
    * <li>bp = this</li>
    * <li>rp = &RP</li>
    * <li>screenUpdate = [this](){submitCommandBuffer("main", 0, populateCommandBufferAccess, this);}</li>
    */
    Renderer(BaseProject* bp, RenderPass* rp, std::function<void()> screenUpdate) {

        this->bp = bp;
        this->rp = rp;
        this->screenUpdate = std::move(screenUpdate);

        for (auto t : allShadersTypes) {
            pipelinesMap.insert({t, std::make_unique<PipelineRenderer>(&localLayout, t)});
            std::cout << getShaderFragName(t) << std::endl;
        }

    }

    ~Renderer() = default;

    /**Load a scene reading a JSON file
     *
     *Note:
     *<li>JSON file must be placed inside "assets/scenes/"</li>
     *<li>When passing the parameter, specify only the name of the file without .json</li>
     */
    void loadSceneFromJSON(std::string sceneName = "test") {
        std::ifstream f("assets/scenes/" + sceneName + ".json");

        if (!f.is_open()) {
            std::cout << "Error! Scene file >" << sceneName << "< not found!";
            exit(-1);
        }
        nlohmann::json data = nlohmann::json::parse(f);

        nlohmann::json mat_info = data["materials"]["LambertBlinnTexture"];
        for (size_t i = 0; i < mat_info.size(); i++) {

            materialAssets.insert({mat_info[i]["id"].get<std::string>(), std::make_unique<LambertTexMaterial>(
                glm::vec3(
                    mat_info[i]["diffuse"][0],
                    mat_info[i]["diffuse"][1],
                    mat_info[i]["diffuse"][2]
                         ),
            glm::vec4(
                    mat_info[i]["specular"][0],
                    mat_info[i]["specular"][1],
                    mat_info[i]["specular"][2],
                    mat_info[i]["specular"][3]
                        ),
                mat_info[i]["texture"].get<std::string>())});
        }

        mat_info = data["materials"]["Water"];
        for (size_t i = 0; i < mat_info.size(); i++) {

            materialAssets.insert({mat_info[i]["id"].get<std::string>(), std::make_unique<WaterMaterial>(
                glm::vec3(
                    mat_info[i]["diffuse"][0],
                    mat_info[i]["diffuse"][1],
                    mat_info[i]["diffuse"][2]
                         ),
            glm::vec4(
                    mat_info[i]["specular"][0],
                    mat_info[i]["specular"][1],
                    mat_info[i]["specular"][2],
                    mat_info[i]["specular"][3]
                    ),
            mat_info[i]["texture"].get<std::string>())});
        }

        mat_info = data["materials"]["Toon"];
        for (size_t i = 0; i < mat_info.size(); i++) {

            materialAssets.insert({mat_info[i]["id"].get<std::string>(), std::make_unique<ToonMaterial>(
                glm::vec3(
                    mat_info[i]["diffuse"][0],
                    mat_info[i]["diffuse"][1],
                    mat_info[i]["diffuse"][2]
                         ),
            glm::vec4(
                    mat_info[i]["specular"][0],
                    mat_info[i]["specular"][1],
                    mat_info[i]["specular"][2],
                    mat_info[i]["specular"][3]
                        ),
                        mat_info[i]["params"][0],
                        mat_info[i]["params"][1],
                        mat_info[i]["params"][2],
                        mat_info[i]["params"][3],
                        mat_info[i]["params"][4],
                        mat_info[i]["params"][5])});
        }

        mat_info = data["instances"];
        sceneObjects.reserve(sceneObjects.size() + mat_info.size());
        for (size_t i = 0; i < mat_info.size(); i++) {
            sceneObjects.emplace_back(dynamic_cast<Model3D*>(mat_info[i]["model"].get<std::string>(),
                glm::vec3(mat_info[i]["position"][0], mat_info[i]["position"][1], mat_info[i]["position"][2]),
                glm::vec3(mat_info[i]["rotation"][0], mat_info[i]["rotation"][1], mat_info[i]["rotation"][2]),
                glm::vec3(mat_info[i]["scale"][0],mat_info[i]["scale"][1],mat_info[i]["scale"][2]),
                materialAssets.at(mat_info[i]["material"].get<std::string>()).get()));

            preLoadModel(sceneObjects[i]);
        }

        std::cout << "RENDERER - JSON PARSED: " << sceneName << std::endl;

    }

    /**Add a new instance of a 3D model
     *
     *NOTE:
     *<li>This method is meant to be called on runtime</li>
     *<li>If you want to load models on advance, use "preLoadModel" instead</li>
     */
    void instantiate(Model3D *model) {
        sceneObjects.push_back(model);

        Model3D* model3D = sceneObjects.back();

        //Key doesn't exist
        try {
            if (modelAssets.find(model3D->getModelPath()) == modelAssets.end()) {
                modelAssets.insert({model3D->getModelPath(), {}});

                Model* m = &modelAssets.at(model3D->getModelPath());
                if (model3D->getModelPath().find(".obj") != std::string::npos)
                    m->init(bp, &vertexDescriptor, "assets/models/" + model3D->getModelPath(),OBJ);
                else if (model3D->getModelPath().find(".gltf") != std::string::npos)
                    m->init(bp, &vertexDescriptor, "assets/models/" + model3D->getModelPath(),GLTF);
                else
                    std::cout << "Error: " <<  model3D->getModelPath() << ", not a valid object file" << std::endl;
            }
        } catch(std::runtime_error& e) {
            // Error is thrown by Starter.hpp if the model does not exist
            error(e.what());
        }

        if ( albedoTexAssets.find(model3D->getMaterial()->getTextureName()) == albedoTexAssets.end()) {
            albedoTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &albedoTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/albedo_" + model3D->getMaterial()->getTextureName());
        }

        if ( armTexAssets.find(model3D->getMaterial()->getTextureName()) == armTexAssets.end()) {
            armTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &armTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/arm_" + model3D->getMaterial()->getTextureName());
        }

        if ( normalTexAssets.find(model3D->getMaterial()->getTextureName()) == normalTexAssets.end()) {
            normalTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &normalTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/normal_" + model3D->getMaterial()->getTextureName());
        }


        //Insert model and texture
        model3D->setModel(&modelAssets[model3D->getModelPath()]);
        model3D->getMaterial()->setAlbedoTex(&albedoTexAssets[model3D->getMaterial()->getTextureName()]);
        model3D->getMaterial()->setArmTex(&armTexAssets[model3D->getMaterial()->getTextureName()]);
        model3D->getMaterial()->setNormalTex(&normalTexAssets[model3D->getMaterial()->getTextureName()]);

        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);

        //Model local descriptor set init
        pipelinesMap.at(model3D->getShaderType())->modelDescriptorSetInit(bp, model3D, &RPoffScreen, &RPsceneDepth, &RPsceneColor);

        queueScreenUpdate();
    }

    ///add a new point light on screen
    void addPointLight(PointLight *light) {
        pointlights.push_back(light);
    }

    void removePointLight(PointLight *light) {
        pointlights.erase(std::remove(pointlights.begin(), pointlights.end(), light), pointlights.end());
    }

    ///add a new spotlight on screen
    void addSpotLight(SpotLight *light) {
        spotlights.push_back(light);
    }

    void removeSpotLight(SpotLight *light) {
        spotlights.erase(std::remove(spotlights.begin(), spotlights.end(), light), spotlights.end());
    }

    ///Create the directional light (Only one directional light can exists)
    void createDirectionalLight(DirectionalLight *light) {
        if (directionalLight) {
            warning(std::format("Trying to set [{}] as Directional Light, but [{}] is already the Directional Light. Ignoring...", directionalLight->UUID, light->UUID));
            return;
        }
        directionalLight = light;
    }

    void unsetDirectionalLight() {
        directionalLight = nullptr;
    }

    /**Set ambient light parameters
     *<li>upper: sky color</li>
     *<li>lower: ground reflection</li>
     *<li>dir: world direction</li>
     */
    void setAmbientLight(AmbientLight *light) {
        if (ambientLight) {
            warning(std::format("Trying to set [{}] as Ambient Light, but [{}] is already the Ambient Light. Ignoring...", ambientLight->UUID, light->UUID));
            return;
        }
        ambientLight = light;
    }

    void unsetAmbientLight() {
        ambientLight = nullptr;
    }

    ///Turn off light that are too far away
    void updateLightCulling(glm::vec3 camPos, float maxDist) {
        for (auto& o : pointlights) {
            o->isOn = glm::distance(o->getLocalPosition(), camPos) < maxDist;
        }

        for (auto& o : spotlights) {
            o->isOn = glm::distance(o->getLocalPosition(), camPos) < maxDist;
        }
    }

    /** Add a new model on the scene
     *
     * NOTE:
     * <li>This method must be called at the top Engine.LocalInit()</li>
     * <li>This method must be used outside of this class only if you don't want to use "loadSceneFromJSON"</li>
     * <li>If you want to add models on runtime use "instantiate" instead</li>
     */
    void preLoadModel(Model3D* model3D) {

        //Key doesn't exist
        if (modelAssets.find(model3D->getModelPath()) == modelAssets.end())
            modelAssets.insert({model3D->getModelPath(), {}});
          if ( albedoTexAssets.find(model3D->getMaterial()->getTextureName()) == albedoTexAssets.end()) {
            albedoTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &albedoTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/albedo_" + model3D->getMaterial()->getTextureName());
        }

        if ( armTexAssets.find(model3D->getMaterial()->getTextureName()) == armTexAssets.end()) {
            armTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &armTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/arm_" + model3D->getMaterial()->getTextureName());
        }

        if ( normalTexAssets.find(model3D->getMaterial()->getTextureName()) == normalTexAssets.end()) {
            normalTexAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &normalTexAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/normal_" + model3D->getMaterial()->getTextureName());
        }

        //Insert model and texture
        model3D->setModel(&modelAssets[model3D->getModelPath()]);
        model3D->getMaterial()->setAlbedoTex(&albedoTexAssets[model3D->getMaterial()->getTextureName()]);
        model3D->getMaterial()->setArmTex(&armTexAssets[model3D->getMaterial()->getTextureName()]);
        model3D->getMaterial()->setNormalTex(&normalTexAssets[model3D->getMaterial()->getTextureName()]);


        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);
    }

    void setObjectVisibility(int i, bool visible) {
        sceneObjects[i]->setIsVisible(visible);

        queueScreenUpdate();
    }

    /** Queues an update to the screen */
    void queueScreenUpdate() {
        this->screenDirty = true;
    }

    /** Flushes the screen if dirty; else does nothing */
    void flushScreenUpdate() {
        if (this->screenDirty) {
            screenUpdate();
            this->screenDirty = false;
        }
    }


    ///This method prepare stuff for Vulkan, must be called inside Engine.localInit()
    void localInit() {
        vertexDescriptor.init(bp,
           {			// number of "bindings" that this vertex uses
               {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}	// binding number, size, and type
           }, {		// this must match the structure Vertex defined above
                 {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                        sizeof(glm::vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                     sizeof(glm::vec3), NORMAL},
                 {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                     sizeof(glm::vec2), UV},
                 {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tan),
                sizeof(glm::vec4), TANGENT}
           });

        VDskybox.init(bp, {
              {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
              {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos), sizeof(glm::vec3), POSITION}
            });

        localLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(UniformBufferObject), 1},
            {1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,0,1}, //albedo
            {2,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,1,1}, //arm
            {3,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,2,1}, //normalMap
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1}, //shadowMap
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1}, //sceneDepth
            {6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5, 1} //sceneColor
          });

        globalLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
          });

        offScreenLayout.init(bp, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(ShadowMapUniformBufferObject), 1}
                    });

        sceneDepthLayout.init(bp, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(SceneDepthUniformBufferObject), 1}
                    });

        sceneColorLayout.init(bp, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(SceneColorUniformBufferObject), 1}
                    });

        skyboxLayout.init(bp, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT,   sizeof(SkyboxUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}    // envMap (samplerCube)
                  });


        for (auto& m : modelAssets) {
            if (m.first.find(".obj") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,OBJ);
            else if (m.first.find(".gltf") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,GLTF);
            else
                std::cout << "Error: " <<  m.first << ", not a valid object file" << std::endl;
        }

        for (auto& t : albedoTexAssets) {
            t.second.init(bp,"assets/textures/albedo_" + t.first);
        }

        for (auto& t : armTexAssets) {
            t.second.init(bp,"assets/textures/arm_" + t.first, VK_FORMAT_R8G8B8A8_UNORM);
        }

        for (auto& t : normalTexAssets) {
            t.second.init(bp,"assets/textures/normal_" + t.first, VK_FORMAT_R8G8B8A8_UNORM);
        }

        for (auto& p : pipelinesMap) {
            p.second->localInit(bp, globalLayout, offScreenLayout, vertexDescriptor);
        }

        float shadowMapSize = 2048;
        RPoffScreen.init(bp, shadowMapSize, shadowMapSize, 1,
                    RenderPass::getStandardAttchmentsProperties(AT_DEPTH_ONLY, bp),
                    RenderPass::getStandardDependencies(ATDEP_DEPTH_TRANS), true);

        //TODO: temp
        float sceneDepthSize = 800;
        RPsceneDepth.init(bp, sceneDepthSize, sceneDepthSize, 1,
                    RenderPass::getStandardAttchmentsProperties(AT_DEPTH_ONLY, bp),
                    RenderPass::getStandardDependencies(ATDEP_DEPTH_TRANS), true);

        float sceneColorSize = 800;
        RPsceneColor.init(bp, sceneColorSize, sceneColorSize, 1,
                    RenderPass::getStandardAttchmentsProperties(AT_ONE_COLOR_AND_DEPTH, bp),
                    RenderPass::getStandardDependencies(ATDEP_DEPTH_TRANS), true);



        PoffScreen.init(bp, &vertexDescriptor, "shaders/ShadowMap.vert.spv", "shaders/ShadowMap.frag.spv", {&offScreenLayout, &localLayout});
        PsceneDepth.init(bp, &vertexDescriptor, "shaders/SceneDepth.vert.spv", "shaders/SceneDepth.frag.spv", {&sceneDepthLayout, &localLayout});
        PsceneColor.init(bp, &vertexDescriptor, "shaders/SceneColor.vert.spv", "shaders/SceneColor.frag.spv", {&sceneColorLayout, &localLayout});

        Pskybox.init(bp, &VDskybox, "shaders/Skybox.vert.spv", "shaders/Skybox.frag.spv", {&skyboxLayout} );
        Pskybox.setCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
        Pskybox.setCullMode(VK_CULL_MODE_FRONT_BIT);   // we are inside the cube
        SkyboxCube.init(bp, &vertexDescriptor, "assets/models/Cube.gltf",     GLTF);


        TenvMap.initCubic(bp, {
            "assets/textures/skybox/px.png",
            "assets/textures/skybox/nx.png",
            "assets/textures/skybox/py.png",
            "assets/textures/skybox/ny.png",
            "assets/textures/skybox/pz.png",
            "assets/textures/skybox/nz.png"
        }, VK_FORMAT_R8G8B8A8_UNORM);
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.descriptorSetsInits()
    void descriptorSetsInits() {

        RPoffScreen.create();
        RPsceneDepth.create();
        RPsceneColor.create();

        global.init(bp, &globalLayout, {});
        offScreen.init(bp, &offScreenLayout, {});
        sceneDepth.init(bp, &sceneDepthLayout, {});
        sceneColor.init(bp, &sceneColorLayout, {});

        skybox.init(bp, &skyboxLayout, {
            TenvMap.getViewAndSampler()
        });
        PoffScreen.create(&RPoffScreen);
        PsceneDepth.create(&RPsceneDepth);
        PsceneColor.create(&RPsceneColor);

        Pskybox.create(rp);

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsInits(bp, rp, &RPoffScreen, &RPsceneDepth, &RPsceneColor);
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.descriptorSetsCleanup()
    void descriptorSetsCleanup() {

        global.cleanup();
        offScreen.cleanup();
        sceneDepth.cleanup();
        sceneColor.cleanup();
        skybox.cleanup();

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsCleanup();
        }

        RPoffScreen.cleanup();
        RPsceneDepth.cleanup();
        RPsceneColor.cleanup();

        PoffScreen.cleanup();
        PsceneDepth.cleanup();
        PsceneColor.cleanup();
        Pskybox.cleanup();
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.localCleanup()
    void localCleanup() {

        globalLayout.cleanup();
        offScreenLayout.cleanup();
        sceneDepthLayout.cleanup();
        sceneColorLayout.cleanup();
        skyboxLayout.cleanup();

        localLayout.cleanup();

        TenvMap.cleanup();
        vertexDescriptor.cleanup();
        VDskybox.cleanup();
        SkyboxCube.cleanup();

        for (auto& m : modelAssets) {
            m.second.cleanup();
        }

        for (auto& t : albedoTexAssets) {
            t.second.cleanup();
        }
        for (auto& t : armTexAssets) {
            t.second.cleanup();
        }

        for (auto& t : normalTexAssets) {
            t.second.cleanup();
        }


        PoffScreen.destroy();
        PsceneDepth.destroy();
        PsceneColor.destroy();
        Pskybox.destroy();
        for (auto& p : pipelinesMap) {
            p.second->localCleanup();
        }

    }

    void populateShadowCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        RPoffScreen.begin(commandBuffer, 0);
        PoffScreen.bind(commandBuffer);
        offScreen.bind(commandBuffer, PoffScreen, 0, currentImage);

        for (auto& o : sceneObjects) {
            o->populateCommandBuffer(commandBuffer, currentImage, PoffScreen);
        }

        RPoffScreen.end(commandBuffer);
    }

    void populateSceneColorCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        RPsceneColor.begin(commandBuffer, 0);
        PsceneColor.bind(commandBuffer);
        sceneColor.bind(commandBuffer, PsceneColor, 0, currentImage);

        for (auto& o : sceneObjects) {
            if (o->getShaderType() != ShaderType::WATER)
                o->populateCommandBuffer(commandBuffer, currentImage, PsceneColor);
        }

        RPsceneColor.end(commandBuffer);
    }

    void populateSceneDepthCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        RPsceneDepth.begin(commandBuffer, 0);
        PsceneDepth.bind(commandBuffer);
        sceneDepth.bind(commandBuffer, PsceneDepth, 0, currentImage);

        for (auto& o : sceneObjects) {
            if (o->getShaderType() != ShaderType::WATER)
            o->populateCommandBuffer(commandBuffer, currentImage, PsceneDepth);
        }

        RPsceneDepth.end(commandBuffer);
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.populateCommandBuffer()
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

        Pskybox.bind(commandBuffer);
        skybox.bind(commandBuffer, Pskybox, 0, currentImage);

        SkyboxCube.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(SkyboxCube.indices.size()), 1, 0, 0, 0);

        for (auto& p : pipelinesMap) {
            if (!IsLateDraw(p.first) )
                p.second->populateCommandBuffer(commandBuffer, currentImage, global, offScreen);
        }

        //Late draw for transparency
        for (auto& p : pipelinesMap) {
            if (IsLateDraw(p.first) )
                p.second->populateCommandBuffer(commandBuffer, currentImage, global, offScreen);
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.updateUniformBuffer()
    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View, float time) {
        GlobalUniformBufferObject gubo;

        gubo.ambientUpper = ambientLight->upper;
        gubo.ambientLower = ambientLight->lower;
        gubo.ambientDir   = ambientLight->dir;

        gubo.lightDir   = glm::vec4(directionalLight->getGlobalRotation(), 0.0f);
        gubo.lightColor = glm::vec4(directionalLight->color, 0.0f) * (float)directionalLight->radiance;

        int j = 0;
        for (size_t i = 0; i < pointlights.size(); i++) {
            if (pointlights[i]->isOn) {
                gubo.pointLightPos[j]    = glm::vec4(pointlights[i]->getGlobalPosition(), 0.0f);
                gubo.pointLightColor[j]  = glm::vec4(pointlights[i]->color, 0.0f) * (float)pointlights[i]->radiance;
                gubo.pointLightParams[j] = glm::vec4(pointlights[i]->decay, (float)pointlights[i]->radius, 0.0f, 0.0f);
                j++;
            }
        }

        gubo.pointInstanceCount = j;

        j = 0;
        for (size_t i = 0; i < spotlights.size(); i++) {
            if (spotlights[i]->isOn) {
                gubo.spotLightPos[j]    = glm::vec4(spotlights[i]->getGlobalPosition(), 0.0f);
                gubo.spotLightDir[j]    = glm::vec4(spotlights[i]->getGlobalRotation(), 0.0f);
                gubo.spotLightColor[j]  = glm::vec4(spotlights[i]->color, 0.0f) * (float)spotlights[i]->radiance;
                gubo.spotLightParams[j] = glm::vec4(
                    cos(glm::radians((float)spotlights[i]->aperture)),   // cIN  = cos(alpha_IN/2)
                    cos(glm::radians((float)spotlights[i]->decay)),    // cOUT = cos(alpha_OUT/2)
                    0.0f,
                    0.0f
                    );
                j++;
            }
        }

        gubo.spotInstanceCount = j;

        // now the eye position corresponds to the position of the camera
        gubo.eyePos = glm::vec4(CamPos,0.0f);
        gubo.time = time;

        // transfers the data to the GPU, by mapping it to its
        // descriptor set
        global.map(currentImage, &gubo, 0);

        // compute shadow MVP early so it's available for the character shadow pass

        const glm::mat4 lightView = glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                    glm::rotate(glm::mat4(1), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::vec3 lightDir =  glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

        ShadowMapUniformBufferObject subo{};
        const float hw = 24.0f;
        const float vw = 24.0f;
        const float lightN = -24.0f;
        const float lightF =  24.0f;
        const glm::mat4 offVP =
                      glm::ortho(-hw, hw, vw, -vw, lightN, lightF) *
                      glm::inverse(lightView);
        subo.mvpMat = offVP;
        offScreen.map(currentImage, &subo, 0);

        SceneDepthUniformBufferObject sdubo{};
        sdubo.mvpMat = Projection*View;
        sceneDepth.map(currentImage, &sdubo, 0);

        SceneColorUniformBufferObject scubo{};
        scubo.mvpMat = Projection*View;
        sceneColor.map(currentImage, &scubo, 0);

        SkyboxUniformBufferObject skyboxUBO{};

        skyboxUBO.mvpMat = Projection * glm::mat4(glm::mat3(View)) * glm::scale(glm::mat4(1), glm::vec3(50.0f));

        skybox.map(currentImage, &skyboxUBO, 0);

        for (auto& p : pipelinesMap) {
            p.second->updateUniformBuffer(currentImage, CamPos, Projection, View);
        }
    }

    ///Return an instance of a model inside sceneObjects
    Model3D* getObject(int i) {
        return sceneObjects[i];
    }

    ///Remove an object from screen and memory
    void removeObject(size_t i) {
        if (i < 0 || i >= sceneObjects.size()) {
            std::cout << "RENDERER - no object found at " << i << " index (removeObject function)" << std::endl;
            return;
        }

        // NOTE: if the game randomly freezes after deleting models this is probably the cause
        // (it works for now, but I'll keep the note as a reminder)
        // tbh this is still buggy
        // sceneObjects[i]->descriptorSetCleanup();
        Model3D* model = sceneObjects[i];

        pipelinesMap.at(model->getShaderType())->removeModel3D(model);

        // model->descriptorSetCleanup();
        sceneObjects.erase(sceneObjects.begin() + i);

        queueScreenUpdate();
    }

    /** Returns the index of the model (or -1 if the object cannot be found) */
    int findObject(Model3D *model) {
        for (size_t i = 0; i < sceneObjects.size(); i++)
            if (model == sceneObjects[i]) 
                return i;

        return -1;
    }

    /** Removes an object, given the the Model3D */
    void removeObject(Model3D *model) {
        int objID = findObject(model);
        if (objID == -1) {
            error("Object not found!");
        }

        this->removeObject(objID);
    }

    size_t getTotalObjectCount() {
        size_t total = 0;
        for (auto& p : pipelinesMap)
            total += p.second->poolSize();
        return total;
    }

};
#endif
