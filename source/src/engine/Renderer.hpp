#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP
#include <unordered_map>

#include "common.h"
#include "Light.hpp"
#include "Model3D.hpp"

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
        DescriptorSetLayout localLayout;
        Pipeline pipeline;

        public:

        PipelineRenderer(ShaderType type = ShaderType::LAMBERT_BLINN) : shaderType(type) {
            vertShader = "shaders/" + getShaderVertName(shaderType) + ".vert.spv";
            fragShader = "shaders/" + getShaderFragName(shaderType) + ".frag.spv";
        }
        ~PipelineRenderer() = default;

        ///Must be called inside Renderer.localInit()
        void localInit(BaseProject* bp, DescriptorSetLayout& globalLayout, VertexDescriptor& vertexDescriptor) {

            localLayout.init(bp, {
                        // this array contains the binding:
                        // first  element : the binding number
                        // second element : the type of element (buffer or texture)
                        // third  element : the pipeline stage where it will be used
                        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(UniformBufferObject), 1},
                        {1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS,0,1}
                      });

            pipeline.init(bp, &vertexDescriptor, vertShader, fragShader, {&globalLayout, &localLayout});
        }

        ///Must be called inside Renderer.descriptorSetsInits()
        void descriptorSetsInits(BaseProject* bp, RenderPass* rp) {
            pipeline.create(rp);

            for (auto& p : pool) {
                modelDescriptorSetInit(bp, p);
            }
        }

        //TODO: non so se mi piace questa soluzione
        void modelDescriptorSetInit(BaseProject* bp, Model3D* model) {
            model->descriptorSetInit(bp, &localLayout);
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
            localLayout.cleanup();
        }

        ///Must be called inside Renderer.populateCommandBuffer()
        void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, DescriptorSet& global) {
            pipeline.bind(commandBuffer);
            global.bind(commandBuffer, pipeline, 0, currentImage);
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
    };

    //Vulkan variables
    DescriptorSetLayout globalLayout;
    DescriptorSet global;
    VertexDescriptor vertexDescriptor;

    //Variable taken from engine
    BaseProject* bp;
    RenderPass* rp;

    //Collections to optimize memory usage
    std::unordered_map<ShaderType, std::unique_ptr<PipelineRenderer>> pipelinesMap;
    std::unordered_map <std::string, Model> modelAssets;
    std::unordered_map <std::string, Texture> textureAssets;
    std::unordered_map <std::string, std::unique_ptr<Material>> materialAssets;
    std::vector<std::unique_ptr<Model3D>> sceneObjects; //stores all the models assigned to Renderer

    DirectionalLight directionalLight;
    std::vector<std::unique_ptr<PointLight>> pointlights;
    std::vector<std::unique_ptr<Spotlight>> spotlights;

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
            pipelinesMap.insert({t, std::make_unique<PipelineRenderer>(t)});
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
        for (int i = 0; i < mat_info.size(); i++) {

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

        mat_info = data["materials"]["Toon"];
        for (int i = 0; i < mat_info.size(); i++) {

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
        for (int i = 0; i < mat_info.size(); i++) {
            sceneObjects.emplace_back(std::make_unique<Model3D>(mat_info[i]["model"].get<std::string>(),
                glm::vec3(mat_info[i]["position"][0], mat_info[i]["position"][1], mat_info[i]["position"][2]),
                glm::vec3(mat_info[i]["rotation"][0], mat_info[i]["rotation"][1], mat_info[i]["rotation"][2]),
                glm::vec3(mat_info[i]["scale"][0],mat_info[i]["scale"][1],mat_info[i]["scale"][2]),
                materialAssets.at(mat_info[i]["material"].get<std::string>()).get()));

            preLoadModel(sceneObjects[i].get());
        }

        std::cout << "RENDERER - JSON PARSED: " << sceneName << std::endl;

    }

    /**Add a new instance of a 3D model
     *
     *NOTE:
     *<li>This method is meant to be called on runtime</li>
     *<li>If you want to load models on advance, use "preLoadModel" instead</li>
     */
    void instantiate(std::string modelPath, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Material* material) {
        sceneObjects.emplace_back(std::make_unique<Model3D>( modelPath, position, rotation, scale, material));

        Model3D* model3D = sceneObjects.back().get();

        //Key doesn't exist
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
        if ( textureAssets.find(model3D->getMaterial()->getTextureName()) == textureAssets.end()) {
            textureAssets.insert({model3D->getMaterial()->getTextureName(),{}});
            Texture* t = &textureAssets.at(model3D->getMaterial()->getTextureName());
            t->init(bp,"assets/textures/" + model3D->getMaterial()->getTextureName());
        }

        //Insert model and texture
        model3D->setModel(&modelAssets[model3D->getModelPath()]);
        model3D->getMaterial()->setTexture(&textureAssets[model3D->getMaterial()->getTextureName()]);


        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);

        //Model local descriptor set init
        pipelinesMap.at(model3D->getShaderType())->modelDescriptorSetInit(bp, model3D);

        screenUpdate();
    }

    ///add a new point light on screen
    void addPointLight(glm::vec3 position ,float radiance, glm::vec3 color, float radius, float decay) {
        pointlights.emplace_back(std::make_unique<PointLight>(position,radiance,color,radius, decay));
    }

    ///add a new spotlight on screen
    void addSpotLight(glm::vec3 position ,float radiance, glm::vec3 color, float aperture, float decay, glm::vec3 direction) {
        spotlights.emplace_back(std::make_unique<Spotlight>(position, radiance,color,aperture, decay, direction));
    }

    ///Create the directional light (Only one directional light can exists)
    void createDirectionalLight( float radiance, glm::vec3 color, glm::vec3 direction) {
        directionalLight = DirectionalLight(radiance, color, direction);
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
        if ( textureAssets.find(model3D->getMaterial()->getTextureName()) == textureAssets.end())
            textureAssets.insert({model3D->getMaterial()->getTextureName(),{}});

        //Insert model and texture
        model3D->setModel(&modelAssets[model3D->getModelPath()]);
        model3D->getMaterial()->setTexture(&textureAssets[model3D->getMaterial()->getTextureName()]);


        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);
    }

    void setObjectVisibility(int i, bool visible) {
        sceneObjects[i]->setIsVisible(visible);

        screenUpdate();
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
                     sizeof(glm::vec2), UV}
           });

        globalLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
          });


        for (auto& m : modelAssets) {
            if (m.first.find(".obj") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,OBJ);
            else if (m.first.find(".gltf") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,GLTF);
            else
                std::cout << "Error: " <<  m.first << ", not a valid object file" << std::endl;
        }

        for (auto& t : textureAssets) {
            t.second.init(bp,"assets/textures/" + t.first);
        }


        for (auto& p : pipelinesMap) {
            p.second->localInit(bp, globalLayout, vertexDescriptor);
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.descriptorSetsInits()
    void descriptorSetsInits() {

        global.init(bp, &globalLayout, {});

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsInits(bp, rp);
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.descriptorSetsCleanup()
    void descriptorSetsCleanup() {

        global.cleanup();

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsCleanup();
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.localCleanup()
    void localCleanup() {

        globalLayout.cleanup();

        for (auto& m : modelAssets) {
            m.second.cleanup();
        }

        for (auto& t : textureAssets) {
            t.second.cleanup();
        }

        for (auto& p : pipelinesMap) {
            p.second->localCleanup();
        }

    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.populateCommandBuffer()
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        for (auto& p : pipelinesMap) {
            p.second->populateCommandBuffer(commandBuffer, currentImage, global);
        }
    }

    ///This method prepare stuff for Vulkan, must be called inside Engine.updateUniformBuffer()
    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {
        GlobalUniformBufferObject gubo;

        gubo.lightDir   = glm::vec4(directionalLight.direction, 0.0f);
        gubo.lightColor = glm::vec4(directionalLight.color, 0.0f) * directionalLight.radiance;

        for (int i = 0; i < pointlights.size(); i++) {
            gubo.pointLightPos[i]    = glm::vec4(pointlights[i].get()->position, 0.0f);
            gubo.pointLightColor[i]  = glm::vec4(pointlights[i].get()->color, 0.0f) * pointlights[i].get()->radiance;
            gubo.pointLightParams[i] = glm::vec4(pointlights[i].get()->decay, pointlights[i].get()->radius, 0.0f, 0.0f);
        }

        gubo.pointInstanceCount = pointlights.size();

        for (int i = 0; i < spotlights.size(); i++) {
            gubo.spotLightPos[i]    = glm::vec4(spotlights[i].get()->position, 0.0f);
            gubo.spotLightDir[i]    = glm::vec4(spotlights[i].get()->direction,0.0f);
            gubo.spotLightColor[i]  = glm::vec4(spotlights[i].get()->color, 0.0f) * spotlights[i].get()->radiance;
            gubo.spotLightParams[i] = glm::vec4(
                cos(glm::radians(spotlights[i].get()->aperture)),   // cIN  = cos(alpha_IN/2)
                cos(glm::radians(spotlights[i].get()->decay)),    // cOUT = cos(alpha_OUT/2)
                0.0f,
                0.0f
                );
        }

        gubo.spotInstanceCount = spotlights.size();

        // now the eye position corresponds to the position of the camera
        gubo.eyePos = glm::vec4(CamPos,0.0f);

        // transfers the data to the GPU, by mapping it to its
        // descriptor set
        global.map(currentImage, &gubo, 0);

        for (auto& p : pipelinesMap) {
            p.second->updateUniformBuffer(currentImage, CamPos, Projection, View);
        }
    }

    ///Return an instance of a model inside sceneObjects
    Model3D* getObject(int i) {
        return sceneObjects[i].get();
    }

    ///Remove an object from screen and memory
    void removeObject(int i) {
        if (i < 0 || i >= sceneObjects.size()) {
            std::cout << "RENDERER - no object found at " << i << " index (removeObject function)" << std::endl;
            return;
        }

        sceneObjects[i].get()->descriptorSetCleanup();
        sceneObjects.erase(sceneObjects.begin() + i);
    }


};
#endif