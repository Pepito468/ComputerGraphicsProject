#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP
#include <unordered_map>

#include "common.h"
#include "PipelineRenderer.hpp"

class Renderer {

    DescriptorSetLayout globalLayout;
    DescriptorSet global;

    VertexDescriptor vertexDescriptor;

    std::unordered_map<ShaderType, std::unique_ptr<PipelineRenderer>> pipelinesMap;
    std::unordered_map <std::string, Model> modelsAssets;
    std::unordered_map <std::string, Texture> texturesAssets;
    std::unordered_map <std::string, std::unique_ptr<Material>> materialsAssets;



    public:
    std::vector<std::unique_ptr<Model3D>> sceneObjects;
    Renderer() {
        /*
        for (auto t : allShadersTypes) {
            pipelinesMap.insert({t, new PipelineRenderer(t)});
            std::cout << getShaderFragName(t) << std::endl;
        }
        */
    }

    ~Renderer() = default;

    void loadSceneFromJSON(std::string sceneName = "test") {
        std::ifstream f("assets/scenes/" + sceneName + ".json");

        if (!f.is_open()) {
            std::cout << "Error! Scene file >" << sceneName << "< not found!";
            exit(-1);
        }
        nlohmann::json data = nlohmann::json::parse(f);

        nlohmann::json mat_info = data["materials"]["LambertBlinnTexture"];
        for (int i = 0; i < mat_info.size(); i++) {

            materialsAssets.insert({mat_info[i]["id"].get<std::string>(), std::make_unique<LambertTexMaterial>(
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

            materialsAssets.insert({mat_info[i]["id"].get<std::string>(), std::make_unique<ToonMaterial>(
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
            enterScene( mat_info[i]["model"].get<std::string>(),
                glm::vec3(mat_info[i]["position"][0], mat_info[i]["position"][1], mat_info[i]["position"][2]),
                glm::vec3(mat_info[i]["rotation"][0], mat_info[i]["rotation"][1], mat_info[i]["rotation"][2]),
                glm::vec3(mat_info[i]["scale"][0],mat_info[i]["scale"][1],mat_info[i]["scale"][2]),
                materialsAssets.at(mat_info[i]["material"].get<std::string>()).get());

            instantiate(sceneObjects[i].get());
        }

        std::cout << "RENDERER - JSON PARSED: " << sceneName << std::endl;

        //exit(0);

    }

    Model3D* getObject(int i) {
        return sceneObjects[i].get();
    }

    void removeObject(int i) {
        if (i < 0 || i >= sceneObjects.size()) {
            std::cout << "RENDERER - no object found at " << i << " index (removeObject function)" << std::endl;
            return;
        }

        if (pipelinesMap.at(sceneObjects[i].get()->getShaderType())->removeModel3D(sceneObjects[i].get())) {
            pipelinesMap.at(sceneObjects[i].get()->getShaderType())->descriptorSetsCleanup();
            pipelinesMap.at(sceneObjects[i].get()->getShaderType())->localCleanup();

            pipelinesMap.erase(sceneObjects[i].get()->getShaderType());
        }

        sceneObjects[i].get()->descriptorSetCleanup();
        sceneObjects.erase(sceneObjects.begin() + i);
    }

    ///Draw a 3D model on the screen
    void enterScene(std::string modelPath, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Material* material) {
        sceneObjects.emplace_back(std::make_unique<Model3D>( modelPath, position, rotation, scale, material));
    }
    void instantiate(Model3D* model3D) {

        //Key doesn't exist
        if (modelsAssets.find(model3D->getModelPath()) == modelsAssets.end())
            modelsAssets.insert({model3D->getModelPath(), {}});
        if ( texturesAssets.find(model3D->getMaterial()->getTextureName()) == texturesAssets.end())
            texturesAssets.insert({model3D->getMaterial()->getTextureName(),{}});
        if (pipelinesMap.find(model3D->getShaderType()) == pipelinesMap.end())
            pipelinesMap.insert({model3D->getShaderType(), std::make_unique<PipelineRenderer>(model3D->getShaderType())});

        //Insert model and texture
        model3D->model = &modelsAssets[model3D->getModelPath()];
        model3D->getMaterial()->texture = &texturesAssets[model3D->getMaterial()->getTextureName()];


        //Add to existing pipeline
        pipelinesMap.at(model3D->getShaderType())->addModel3D(model3D);
    }

    void instantiate(Model3D* model3D, BaseProject* bp,  RenderPass* rp) {
        instantiate(model3D);

        //Model descriptor set init
        pipelinesMap.at(model3D->getShaderType())->modelDescriptorSetInit(bp, model3D);
    }

    void localInit(BaseProject* bp) {
        vertexDescriptor.init(bp,
           {			// number of "bindings" that this vertex uses
               {0, sizeof(SimpleVertex), VK_VERTEX_INPUT_RATE_VERTEX}	// binding number, size, and type
           }, {		// this must match the structure Vertex defined above
                 {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, pos),
                        sizeof(glm::vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, norm),
                     sizeof(glm::vec3), NORMAL},
                 {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(SimpleVertex, UV),
                     sizeof(glm::vec2), UV}
           });

        globalLayout.init(bp, {
            // this array contains the binding:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
          });

        for (auto& m : modelsAssets) {
            if (m.first.find(".obj") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,OBJ);
            else if (m.first.find(".gltf") != std::string::npos)
                m.second.init(bp, &vertexDescriptor, "assets/models/" + m.first,GLTF);
            else
                std::cout << "Error: " <<  m.first << ", not a valid object file" << std::endl;
        }

        for (auto& t : texturesAssets) {
            t.second.init(bp,"assets/textures/" + t.first);
        }


        for (auto& p : pipelinesMap) {
            p.second->localInit(bp, globalLayout, vertexDescriptor);
        }
    }

    void descriptorSetsInits(BaseProject* bp, RenderPass* rp) {

        global.init(bp, &globalLayout, {});

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsInits(bp, rp);
        }
    }

    void descriptorSetsCleanup() {

        global.cleanup();

        for (auto& p : pipelinesMap) {
            p.second->descriptorSetsCleanup();
        }
    }

    void localCleanup() {

        globalLayout.cleanup();

        for (auto& m : modelsAssets) {
            m.second.cleanup();
        }

        for (auto& t : texturesAssets) {
            t.second.cleanup();
        }

        for (auto& p : pipelinesMap) {
            p.second->localCleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        for (auto& p : pipelinesMap) {
            p.second->populateCommandBuffer(commandBuffer, currentImage, global);
        }
    }

    void updateUniformBuffer(uint32_t currentImage,  glm::vec3 CamPos, glm::mat4 Projection, glm::mat4 View) {
        //TODO: provvisoro per test
        GlobalUniformBufferObject gubo;

        // fills with the relevant data
        gubo.lightDir = glm::vec3(0.5656854, 0.7071068, 0.4242641);
        gubo.lightColor = glm::vec4(1.0, 1.0, 1.0, 0.0);
        // now the eye position corresponds to the position of the camera
        gubo.eyePos = CamPos;

        // transfers the data to the GPU, by mapping it to its
        // descriptor set
        global.map(currentImage, &gubo, 0);

        for (auto& p : pipelinesMap) {
            p.second->updateUniformBuffer(currentImage, CamPos, Projection, View);
        }
    }


};
#endif