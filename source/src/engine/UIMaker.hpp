#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include "Texture.hpp"
#include "Debug.hpp"

// uncomment to remove errors from missing libraries
// #include <vulkan/vulkan_core.h>
// #include "stb_image.h"
// #define  STARTER_IMPLEMENTATION
// #include "modules/Starter.hpp"

//TODO fix Invalid VkBuffer Object when rendering UIElements

#define DEFAULT_SUBMIT_ORDER 9999

enum UIOriginH { UIO_LEFT, UIO_CENTER, UIO_RIGHT };
enum UIOriginV { UIO_TOP, UIO_MIDDLE, UIO_BOTTOM };

/// Stores the position of the vertex
struct UIVertex {
	glm::vec2 pos;
	glm::vec2 uv;
};

/// Stores the texture, along with its width and height
struct UITextureData {
	int width;
	int height;
	std::vector<Texture> textureVec;
};

class UIMaker {
	struct UIMakerAndModel {
		UIMaker* ui;
		Model* M;
	};

	class UIElement {
	public:
		// Position
		float x, y;
		
		// Scale
		float sx, sy;
		
		/**
		* Position of the origin between the following points of the rectangle:
		* x -- x -- x
		* |         |
		* x    x    x
		* |         |
		* x -- x -- x
		*/
		UIOriginH RegH;
		UIOriginV RegV;

		Pipeline P;
		Model *M = nullptr, *oldM = nullptr; /// oldM stores M in case other processes still have the previous model
		UITextureData T;
		DescriptorSet DS;

		UIElement() {}
		~UIElement() = default;	//TODO implement proper deconstructor

		void addTexture(Texture t) {
			T.textureVec.push_back(t);
		}

		void render(float x, float y, float sx, float sy, UIOriginH RegH, UIOriginV RegV) {
			this->x = x;
			this->y = y;
			this->sx = sx;
			this->sy = sy;
			this->RegH = RegH;
			this->RegV = RegV;
		}

		void createMesh(VertexDescriptor* VD, int screenW, int screenH, BaseProject* BP) {
			/// Old model cleanup
			if (this->M) {
				if (this->oldM) {
					oldM->cleanup();
					delete(oldM);
				}
				this->oldM = this->M;
			}

			int mainStride = sizeof(UIVertex);
			this->M = new Model();
			
			this->M->indices.resize(6);
			this->M->vertices.resize(4 * mainStride);
			// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " mainStride: " << mainStride << std::endl;

			float tpx = 0.0f;
			float tpy = 0.0f;
			
			UIVertex *V_vertex = (UIVertex *)(&this->M->vertices[0]);

			tpx = (this->x + 1.0f)/2.0f * screenW - this->sx * (
				(this->RegH == UIO_RIGHT  ? (float)this->T.width	   : 0.0f) +
				(this->RegH == UIO_CENTER ? (float)this->T.width/2.0f  : 0.0f)
			);
			tpy = (this->y + 1.0f)/2.0f * screenH - this->sy * (
				(this->RegV == UIO_BOTTOM ? (float)this->T.height	   : 0.0f) +
				(this->RegV == UIO_MIDDLE ? (float)this->T.height/2.0f : 0.0f)
			);
			
			makeUIVertex(V_vertex, tpx, tpy, 0, 0, screenW, screenH);
			V_vertex++;

			makeUIVertex(V_vertex, tpx + (float)(this->T.width) * this->sx, tpy, 1, 0, screenW, screenH);
			V_vertex++;
			
			makeUIVertex(V_vertex, tpx, tpy + (float)(this->T.height) * this->sy, 0, 1, screenW, screenH);
			V_vertex++;

			makeUIVertex(V_vertex, tpx + (float)(this->T.width) * this->sx, tpy + (float)(this->T.height) * this->sy, 1, 1, screenW, screenH);
			V_vertex++;
			
			this->M->indices[0] = 0;
			this->M->indices[1] = 1;
			this->M->indices[2] = 2;
			this->M->indices[3] = 1;
			this->M->indices[4] = 2;
			this->M->indices[5] = 3;

			// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " init mesh" << std::endl;
			this->M->initMesh(BP, VD, false);
		}

		void makeUIVertex(UIVertex *V, float px, float py, float tx, float ty, int screenW, int screenH) {
			// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " makeUIVertex: ";
			/// Transforms pixel to screen coordinates
			V->pos.x = (px + 0.5f) / (float)screenW * 2.0f - 1.0f;
			V->pos.y = (py + 0.5f) / (float)screenH * 2.0f - 1.0f;

			V->uv.x = tx;
			V->uv.y = ty;
			// std::cout << "pos = (" << V->pos.x << "," << V->pos.y << ") | uv = (" << V->uv.x << "," << V->uv.y << ")" << std::endl;
		}
		
		/**
		 * Creates the local descriptor set of the UIElement, using textureVec[0] as the default texture
		 */
		void createDescriptorSet(DescriptorSetLayout* DSL, BaseProject* BP) {
			DescriptorSet temp;
			temp.init(BP, DSL, {this->T.textureVec[0].getViewAndSampler()});
			this->DS = temp;
		}

		/**
		 * Recreates the local descript set using textureVec[i] as the texture
		 * Throws an error if i is an invalid index of the vector
		 */
		void recreateDescriptorSet(DescriptorSetLayout *DSL, BaseProject *BP, int i) {
			if (i >= this->T.textureVec.size() || i < 0) {
				error("Tried to access texture out of bounds: " + std::to_string(i) + " | texture vector size " + std::to_string(this->T.textureVec.size()));
			}

			DescriptorSet temp;
			temp.init(BP, DSL, {this->T.textureVec[i].getViewAndSampler()});
			this->DS = temp;
		}
	};

	VertexDescriptor UI_VD;
	BaseProject* BP;
	DescriptorSetLayout UI_DSL;
	RenderPass UI_RP;

	int screenW, screenH;
	int submitOrder;
	
	std::unordered_map<int, UIElement> UIElementsMap = {};
	
	bool commandBufferMustUpdate = false;
	
public:
	UIMaker(BaseProject *_BP) {
		BP = _BP;
	}
	//TODO implement deconstructor
	~UIMaker() = default;

	//--------------------------------------------

	/**
	* Initializes the UIMaker with the given textures, creating a pipeline for each UIElement
	* By default, TextureFiles and TextureDataList are empty lists, and so = DEFAULT_SUBMIT_ORDER
	* NOTE: the id of the UIElement depends on the position in the lists TextureFiles and TextureDataList
	*/
	void init(int sW, int sH, std::list<std::vector<std::string>> TextureFiles = {}, std::list<std::vector<ProceduralTextures::TextureData>> TextureDataList = {}, int so = DEFAULT_SUBMIT_ORDER)  {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI init" << std::endl;
		screenW = sW;
		screenH = sH;
		submitOrder = so;

		int UIElementsNumber = TextureFiles.size() + TextureDataList.size();

		createUIDescriptorSetAndVertexLayout();
		createUIPipeline(UIElementsNumber);

		UI_RP.init(BP, sW, sH, -1, RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, BP));
		UI_RP.properties[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		UI_RP.properties[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		int i = 0, garbage;
		for (auto textureChunk : TextureFiles) {
			for (auto t : textureChunk) {
				Texture temp;
				temp.init(BP, t);

				UIElementsMap[i].addTexture(temp);
			}
			/// Assumes all texture with the same size
			unsigned char* pixels = stbi_load(textureChunk[0].c_str(), &UIElementsMap[i].T.width, &UIElementsMap[i].T.height, &garbage, STBI_rgb_alpha);
			stbi_image_free(pixels);

			i++;
		}

		for (auto textureChunk : TextureDataList) {
			for (auto t : textureChunk) {
				Texture temp;
				temp.initPixels(BP, t.width, t.height, 4, sizeof(uint8_t), {t.pixels.data()});

				UIElementsMap[i].addTexture(temp);
			}

			UIElementsMap[i].T.width = textureChunk[0].width;
			UIElementsMap[i].T.height = textureChunk[0].height;

			i++;
		}

		BP->DPSZs.texturesInPool += UIElementsNumber;
		BP->DPSZs.setsInPool += UIElementsNumber;
	}

	/**
	* Sets up the Vertex Descriptor and Descriptor Set Layout
	*/
	void createUIDescriptorSetAndVertexLayout() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " Create UI descriptor sets and vertex layouts" << std::endl;
		UI_VD.init(BP, {{0, sizeof(UIVertex), }}, {
			{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, pos), sizeof(glm::vec2), POSITION},
			{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, uv), sizeof(glm::vec2), UV}
		});

		UI_DSL.init(BP, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}});
	}

 	/**
	* Sets up the main Pipeline with a default shader
	*/
	void createUIPipeline(int pipelinesNumber) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " Creating " << pipelinesNumber << " UI pipelines with UI_VD = " << &UI_VD << std::endl;
		for (int i = 0; i < pipelinesNumber; i++) {
			UIElementsMap[i].P.init(BP, &UI_VD, "shaders/UIElement.vert.spv", "shaders/UIElement.frag.spv", {&UI_DSL});
			UIElementsMap[i].P.setCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
			UIElementsMap[i].P.setCullMode(VK_CULL_MODE_NONE);
			UIElementsMap[i].P.setTransparency(false);
		}
	}

	/**
	 * Creates a descriptor set for each UIElement
	 */
	void createUIDescriptorSets() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI descritor sets init";
		for (auto& e : UIElementsMap) {
			e.second.createDescriptorSet(&UI_DSL, BP);
		}
	}

	void pipelinesAndDescriptorSetsInit() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI pipelines and descriptor sets init" << std::endl;
		UI_RP.create();

		for (auto& e : UIElementsMap) {
			// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " creating pipeline #" << e.first << std::endl;
			e.second.P.create(&UI_RP);
		}

		createUIDescriptorSets();
	}

	static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " populate command buffer access" << std::endl;
		UIMaker *T = ((UIMakerAndModel *)Params)->ui;
		T->populateCommandBuffer(commandBuffer, currentImage);
	}

	inline void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " populate command buffer" << std::endl;
		UI_RP.begin(commandBuffer, currentImage);

		for (auto& e : UIElementsMap) {
			e.second.P.bind(commandBuffer);
			e.second.M->bind(commandBuffer);
			e.second.DS.bind(commandBuffer, e.second.P, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(e.second.M->indices.size()), 1, 0, 0, 0);
		}
		
		UI_RP.end(commandBuffer);
	}
	
	//--------------------------------------------

	/**
	* Notifies that the UI element with the given id needs to be updated
	* Throws an error if the id isn't present in UIElementsMap, and a warning if either sx or sy are 0
	*/
	int renderUI(float x, float y, int id, UIOriginH RegH = UIO_LEFT, UIOriginV RegV = UIO_TOP, float sx = 1.0f, float sy = 1.0f) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " renderUI id = " << id << std::endl;
		if (sx == 0 || sy == 0)
			warning("1-dimensional UI element: id = " + std::to_string(id) + ", sx = " + std::to_string(sx) + ", sy = " + std::to_string(sy));
		
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		UIElementsMap[id].render(x, y, sx, sy, RegH, RegV);
		commandBufferMustUpdate = true;
		
		return id;
	}

	/**
	* Rearranges UI elements on screen resize
	*/
	void resizeScreen(int sW, int sH) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI resizeScreen" << std::endl;
		screenW = sW;
		screenH = sH;
		UI_RP.width = sW;
		UI_RP.height = sH;

		commandBufferMustUpdate = true;
	}

	/**
	 * If at least a UIElement was changed (commandBufferMustUpdate = true), redraws every UIElement
	 */
	void updateCommandBuffer() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI update command buffer" << std::endl;
		// debugPrint();
		if (commandBufferMustUpdate) {
			//could add an update field to each UIElement to avoid updating unchanged elements, but doesn't seem worth it
			for (auto& e : UIElementsMap) {
				createUIMesh(e.first);	// creates the new mesh
				
				UIMakerAndModel *uim = (UIMakerAndModel *)malloc(sizeof(UIMakerAndModel));
				uim->ui = this;
				uim->M = e.second.M;
				BP->submitCommandBuffer("ui" + std::to_string(e.first), submitOrder,UIMaker::populateCommandBufferAccess, uim, UIMaker::freeCommandBuffer);
			}

			commandBufferMustUpdate = false;
		}
	}

	/**
	* Creates the mesh for the given UI element
	*/
	void createUIMesh(int id) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " createUImesh with id " << id << std::endl;
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));
		
		UIElementsMap[id].createMesh(&UI_VD, screenW, screenH, BP);
	}

	void recreateUIDescriptorSet(int id, int idTexture) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " re-create DS with id " << id << "and texture " << idTexture << std::endl;
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		UIElementsMap[id].recreateDescriptorSet(&UI_DSL, BP, idTexture);
	}

	//--------------------------------------------

	/**
	* Removes a single UI element, given its id
	*/
	void removeUIElement(int id) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " removeUIElement id = " << id << std::endl;
		//TODO implement proper deconstructor
		UIElementsMap.erase(id);
		commandBufferMustUpdate = true;
	}

	/**
	* Removes all UI elements
	*/
	void removeUI() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " removeUI" << std::endl;
		//TODO implement proper deconstructor
		UIElementsMap.clear();
		commandBufferMustUpdate = true;
	}

	void pipelinesAndDescriptorSetsCleanup() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI pipelines and descript sets cleanup" << std::endl;
		for (auto& e : UIElementsMap)
			e.second.P.cleanup();

		UI_RP.cleanup();
		
		for (auto& e : UIElementsMap)
			e.second.DS.cleanup();
	}

	void localCleanup() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " local cleanup" << std::endl;
		for (auto& e : UIElementsMap) 
			for (auto& t : e.second.T.textureVec)
				t.cleanup();
		
		for (auto& e : UIElementsMap) 
			if(e.second.M)
				e.second.M->cleanup();

		UI_DSL.cleanup();
		
		for (auto& e : UIElementsMap) 
			e.second.P.destroy();

		UI_RP.destroy();
	}

	static void freeCommandBuffer(void *Params) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " free command buffer" << std::endl;
		Model *M = ((UIMakerAndModel *)Params)->M;
		M->cleanup();
		
		free(Params);
	}

	//--------------------------------------------

	void debugPrint() {
		std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " DEBUG PRINTING -------------------------------" << std::endl;

		std::cout << "UIElements map size: " << UIElementsMap.size() << std::endl;
		for (auto e : UIElementsMap) {
			std::cout << "[" << e.first << "]" << std::endl;

			std::cout << "\tModel:";
			if(e.second.M) {
				for (auto i : e.second.M->indices) {
					std::cout << " " << i;
				}
			} else {
				std::cout << "empty";
			}
			std::cout << std::endl;

			std::cout << "\tTextures: #" << e.second.T.textureVec.size() << " " << e.second.T.width << " x " << e.second.T.height << std::endl;
		}

		std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " END DEBUG PRINTING ---------------------------" << std::endl;
	}
};
