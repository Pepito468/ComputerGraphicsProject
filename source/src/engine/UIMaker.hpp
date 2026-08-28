#include <cstdint>
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

/*
	TODO list:
	- Main menu
	- Pause menu
		- Settings (maybe)
		- Fullscreen background tint
	- Buttons
	- Sliders and checkboxes (maybe)
	- Resizable ui
		- UI that scale with a fixed aspect ratio
	- Fix leaked objects
*/

#define DEFAULT_SUBMIT_ORDER 9999
#define DEFAULT_WINDOW_WIDTH 1080
#define DEFAULT_WINDOW_HEIGHT 720

#define MAX_OLD_MODELS_BUFFER 3

enum UIOriginH { UIO_LEFT, UIO_CENTER, UIO_RIGHT };
enum UIOriginV { UIO_TOP, UIO_MIDDLE, UIO_BOTTOM };

enum UIElementType { UI_STATIC, UI_BUTTON, UI_SLIDER, UI_CHECKBOX };
enum ResizableType {
	NOT_RESIZABLE,			/// When resizing the window, the UIElement will keep its dimensions constant
	FULL_RESIZABLE,			/// When resizing the window, the UIElement will scale proportionally
	WIDTH_ONLY_RESIZABLE,	/// When resizing the window, the UIElement will only scale horizontally
	HEIGHT_ONLY_RESIZABLE 	/// When resizing the window, the UIElement will only scale vertically
};

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

struct TextureFilesWithParams {
	std::list<std::string> TextureFiles;
	bool isTransparent = false;
	ResizableType resize = NOT_RESIZABLE;
};

struct TextureDataWithParams {
	std::list<GeneratedTextureData> TextureData;
	bool isTransparent = false;
	ResizableType resize = NOT_RESIZABLE;
};

class UIMaker {
	struct UIMakerAndModel {
		UIMaker* ui;
		Model* M;
	};

	class UIElement {
	public:
		/// Position
		float x, y;
		
		/// Scale
		float sx, sy;	/// If the UIElement is scalable, these are relative to a screen sized 1080x720
		
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
		Model *M = nullptr;
		// std::list<Model*> oldM = {};	 /// oldM stores M in case other processes still have the previous model
		UITextureData T;
		DescriptorSet DS;

		UIElement() {}
		~UIElement() = default;

		bool isVisible = true;
		bool isTransparent = false;
		ResizableType resize = NOT_RESIZABLE;

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
			// if (this->M) {
			// 	this->oldM.push_back(this->M);
			// 	if (this->oldM.size() > MAX_OLD_MODELS_BUFFER) {
			// 		delete(this->oldM.front());
			// 		this->oldM.pop_front();
			// 	}
			// }

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

		void scaleToScreen (int screenW, int screenH) {
			if (this->resize == FULL_RESIZABLE || this->resize == WIDTH_ONLY_RESIZABLE)
				this->sx = (float)screenW / DEFAULT_WINDOW_WIDTH;
			
			if (this->resize == FULL_RESIZABLE || this->resize == HEIGHT_ONLY_RESIZABLE)
				this->sy = (float)screenH / DEFAULT_WINDOW_HEIGHT;
		}
	};

	VertexDescriptor UI_VD;
	BaseProject* BP;
	DescriptorSetLayout UI_DSL;
	RenderPass UI_RP;

	int screenW, screenH;	//should be swapchain width and height instead of screen, but can't access it (TextMaker has the same bug)
	int submitOrder;
	
	double mousePosX, mousePosY;
	
	std::unordered_map<int, UIElement> UIElementsMap = {};
	
	bool commandBufferMustUpdate = false;
	
public:
	UIMaker(BaseProject *_BP) {
		BP = _BP;
	}
	
	~UIMaker() = default;

	//--------------------------------------------

	/**
	* Initializes the UIMaker with the given textures, creating a pipeline for each UIElement
	* By default, TextureFiles and TextureDataList are empty lists, and so = DEFAULT_SUBMIT_ORDER
	* NOTE: the id of the UIElement depends on the position in the lists TextureFiles and TextureDataList (in order)
	*/
	void init(int sW, int sH, std::list<TextureFilesWithParams> TextureFilesList = {}, std::list<TextureDataWithParams> TextureDataList = {}, int so = DEFAULT_SUBMIT_ORDER)  {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI init" << std::endl;
		screenW = sW;
		screenH = sH;
		submitOrder = so;

		int UIElementsNumber = TextureFilesList.size() + TextureDataList.size();

		createUIDescriptorSetAndVertexLayout();
		createUIPipeline(UIElementsNumber);

		UI_RP.init(BP, sW, sH, -1, RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, BP));
		UI_RP.properties[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		UI_RP.properties[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		int i = 0, garbage;
		for (auto textureChunk : TextureFilesList) {
			UIElementsMap[i].P.setTransparency(textureChunk.isTransparent);

			for (auto t : textureChunk.TextureFiles) {
				Texture temp;
				temp.init(BP, t);

				UIElementsMap[i].addTexture(temp);
			}
			/// Assumes all texture with the same size
			unsigned char* pixels = stbi_load(textureChunk.TextureFiles.front().c_str(), &UIElementsMap[i].T.width, &UIElementsMap[i].T.height, &garbage, STBI_rgb_alpha);
			stbi_image_free(pixels);

			UIElementsMap[i].resize = textureChunk.resize;
			i++;
		}

		for (auto textureChunk : TextureDataList) {
			UIElementsMap[i].P.setTransparency(textureChunk.isTransparent);

			for (auto t : textureChunk.TextureData) {
				Texture temp;
				temp.initPixels(BP, t.width, t.height, 4, sizeof(uint8_t), {t.pixels.data()});

				UIElementsMap[i].addTexture(temp);
			}

			UIElementsMap[i].T.width = textureChunk.TextureData.front().width;
			UIElementsMap[i].T.height = textureChunk.TextureData.front().height;

			UIElementsMap[i].resize = textureChunk.resize;
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
		UI_VD.init(BP, {{0, sizeof(UIVertex), VK_VERTEX_INPUT_RATE_VERTEX}}, {
			{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, pos), sizeof(glm::vec2), POS2D},
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
			/* TODO to create >1 viewports, need multiViewport feature; is it worth it?
			if (e.second.hasScissor) {
				std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " creating scissor and viewport" << std::endl;
				e.second.P.setScissor({
					{(int32_t)(screenW - e.second.T.width), (int32_t)(screenH - e.second.T.height)},
					{(int32_t)e.second.T.width, (int32_t)e.second.T.height}
				});
				e.second.P.setViewport({{
					(float)(screenW - e.second.T.width), (float)(screenH - e.second.T.height), 
					(float)(e.second.T.width), (float)(e.second.T.height), 0.0f, 1.0f
				}});
			} */
		}

		createUIDescriptorSets();
	}

	static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " populate command buffer access" << std::endl;
		UIMaker *T = ((UIMakerAndModel *)Params)->ui;
		T->populateCommandBuffer(commandBuffer, currentImage);
	}

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " populate command buffer" << std::endl;
		UI_RP.begin(commandBuffer, currentImage);

		for (auto& e : UIElementsMap) {
			if (e.second.isVisible) {
				e.second.P.bind(commandBuffer);
				e.second.M->bind(commandBuffer);
				e.second.DS.bind(commandBuffer, e.second.P, 0, currentImage);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(e.second.M->indices.size()), 1, 0, 0, 0);
			}
		}
		
		UI_RP.end(commandBuffer);
	}

	void setMousePosition(double x, double y) {
		this->mousePosX = x;
		this->mousePosY = y;
	}

	void toggleVisibility(int id) {
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		elem->second.isVisible = !elem->second.isVisible;
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

		for (auto &e : UIElementsMap) {
			if (e.second.resize != NOT_RESIZABLE)
				e.second.scaleToScreen(sW, sH);
		}

		commandBufferMustUpdate = true;
	}

	/**
	 * If at least a UIElement was changed (commandBufferMustUpdate = true), redraws every UIElement.
	 * Also, updates mousePos[X, Y] if the cursor isn't locked
	 */
	void updateCommandBuffer(bool isCursorAvailable) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " UI update command buffer" << std::endl;
		// debugPrint();
		if (isCursorAvailable) {
			//TODO deal with buttons, etc...
		}

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
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning pipelines" << std::endl;
		for (auto& e : UIElementsMap)
			e.second.P.cleanup();
	
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning render pass" << std::endl;
		UI_RP.cleanup();
		
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning descriptor sets" << std::endl;
		for (auto& e : UIElementsMap)
			e.second.DS.cleanup();
	}

	void localCleanup() {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " local cleanup" << std::endl;
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning textures" << std::endl;
		for (auto& e : UIElementsMap) 
			for (auto& t : e.second.T.textureVec)
				t.cleanup();
		
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning models" << std::endl;
		for (auto& e : UIElementsMap) {
			// while (e.second.oldM.size() > 0 && e.second.oldM.front()) {
			// 	delete(e.second.oldM.front());
			// 	e.second.oldM.pop_front();
			// }
			if (e.second.M) {
				e.second.M->cleanup();
				delete(e.second.M);
			}
		}

		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tCleaning descriptor set layout" << std::endl;
		UI_DSL.cleanup();
		
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tDestroying pipelines" << std::endl;
		for (auto& e : UIElementsMap) 
			e.second.P.destroy();

		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << "\tDestroying render pass" << std::endl;
		UI_RP.destroy();
	}

	static void freeCommandBuffer(void *Params) {
		// std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " free command buffer" << std::endl;
		Model *M = ((UIMakerAndModel *)Params)->M;
		M->cleanup();
		delete(M);
		
		free(Params);
	}

	//--------------------------------------------

	void debugPrint() {
		std::cout << COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT << " DEBUG PRINTING -------------------------------" << std::endl;

		std::cout << "UIElements map size: " << UIElementsMap.size() << std::endl;
		for (auto e : UIElementsMap) {
			std::cout << "[" << e.first << "]" << std::endl;

			std::cout << "\tPosition: " << e.second.x << " x " << e.second.y << std::endl;
			std::cout << "\tScale: " << e.second.sx << " x " << e.second.sy << std::endl;

			std::cout << "\tUIOrigin: ";
			switch(e.second.RegH) {
			case UIO_LEFT:
				std::cout << "LEFT";
				break;
			case UIO_CENTER:
				std::cout << "CENTER";
				break;
			case UIO_RIGHT:
				std::cout << "RIGHT";
				break;
			}
			std::cout << " x ";
			switch(e.second.RegV) {
			case UIO_TOP:
				std::cout << "TOP";
				break;
			case UIO_MIDDLE:
				std::cout << "MIDDLE";
				break;
			case UIO_BOTTOM:
				std::cout << "BOTTOM";
				break;
			}
			std::cout << std::endl;

			std::cout << "Resizable type: ";
			switch (e.second.resize) {
			case NOT_RESIZABLE:
				std::cout << "not resizable";
				break;
			case FULL_RESIZABLE:
				std::cout << "resizable";
				break;
			case WIDTH_ONLY_RESIZABLE:
				std::cout << "only width resizable";
				break;
			case HEIGHT_ONLY_RESIZABLE:
				std::cout << "only height resizable";
				break;
			}
			std::cout << std::endl;

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
