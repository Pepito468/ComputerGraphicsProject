#ifndef UIMAKER_H
#define UIMAKER_H

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
	- Add a forceModelUpdate flag to UIMaker and a needsUpdating flag to UIElements (maybe)
	- Checkboxes (equiv 2-stage buttons) (maybe)
	- N-stage buttons (A -> click -> B -> click -> C -> click -> A -> ...) (maybe)
	- Main menu
*/

#define DEFAULT_SUBMIT_ORDER				9999
#define DEFAULT_WINDOW_WIDTH				1080
#define DEFAULT_WINDOW_HEIGHT				720

#define UI_ID_NULL							-1
#define UI_ID_MENU_BACKGROUND				20
#define UI_ID_TITLE							21
#define UI_ID_COMMANDS						75
#define UI_ID_SLIDER_VOLUME_PLAQUE			50
#define UI_ID_SLIDER_VOLUME_BACKGROUND		51
#define UI_ID_SLIDER_SENSITIVITY_PLAQUE		52
#define UI_ID_SLIDER_SENSITIVITY_BACKGROUND	53
#define UI_ID_BUTTON_START					99
#define UI_ID_BUTTON_RESUME					100
#define UI_ID_BUTTON_QUIT					101
#define UI_ID_SLIDER_VOLUME					150
#define UI_ID_SLIDER_SENSITIVITY			151
#define UI_ID_BUTTON_SCENE1					200
#define UI_ID_BUTTON_SCENE2					201

#define UI_DEBUG_STRING COLOR_BRIGHT_GREEN << "[UI DEBUG]" << COLOR_DEFAULT

enum UIOriginH { UIO_LEFT, UIO_CENTER, UIO_RIGHT };
enum UIOriginV { UIO_TOP, UIO_MIDDLE, UIO_BOTTOM };

enum UIElementType { UI_NORMAL, UI_BUTTON, UI_SLIDER };
enum ResizableType {	/// When resizing the window, the UIElement will...
	NOT_RESIZABLE,			/// keep its dimensions constant
	FULL_RESIZABLE,			/// scale proportionally
	WIDTH_ONLY_RESIZABLE,	/// only scale horizontally
	HEIGHT_ONLY_RESIZABLE, 	/// only scale vertically
	KEEP_ASPECT_RATIO		/// scale while keeping its aspect ratio the same
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
	UIElementType type = UI_NORMAL;
};

struct TextureDataWithParams {
	std::list<GeneratedTextureData> TextureData;
	bool isTransparent = false;
	ResizableType resize = NOT_RESIZABLE;
	UIElementType type = UI_NORMAL;
};

struct InteractedUIElementData {
	int id;
	float data;
};

inline bool isPointInsideRectangle(float _x, float _y, float x1, float y1, float x2, float y2) {
	bool inside = true;
			
	// std::cout << "Hitbox: " << x1 << " x " << y1 << " | " << x2 << " x " << y2 << std::endl << "Point: " << _x << " x " << _y << std::endl;
	inside &= ((x1 <= _x) && (_x <= x2));
	inside &= ((y1 <= _y) && (_y <= y2));
	// std::cout << "Inside: " << inside << std::endl;

	return inside;
}

inline bool isPointInsideRectangle(float _x, float _y, glm::vec2 a, glm::vec2 b) {
	return isPointInsideRectangle(_x, _y, a.x, a.y, b.x, b.y);
}

inline std::array<glm::vec2, 2> getPixelCoordinates(int screenW, int screenH, float x, float y, float sx, float sy, UIOriginH RegH, UIOriginV RegV, int width, int height) {
	// std::cout << UI_DEBUG_STRING << " getting coordinates of an element: " << x << " x " << y << "|" << sx << " x " << sy << std::endl;
	float tpx = 0.0f, tpy = 0.0f;
	std::array<glm::vec2, 2> coordinates = {};

	tpx = (x + 1.0f)/2.0f * screenW - sx * (
		(RegH == UIO_RIGHT  ? (float)width		: 0.0f) +
		(RegH == UIO_CENTER ? (float)width/2.0f	: 0.0f)
	);
	tpy = (y + 1.0f)/2.0f * screenH - sy * (
		(RegV == UIO_BOTTOM ? (float)height		: 0.0f) +
		(RegV == UIO_MIDDLE ? (float)height/2.0f: 0.0f)
	);

	coordinates[0].x = tpx;
	coordinates[0].y = tpy;

	coordinates[1].x = tpx + (float)(width) * sx;
	coordinates[1].y = tpy + (float)(height) * sy;

	return coordinates;
}

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
		/// NOTE: negative scale produce mirroring, and might have unintuitive behavior with UIOrigin
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

		bool isVisible = false;
		bool isTransparent = false;
		UIElementType type = UI_NORMAL;
		ResizableType resize = NOT_RESIZABLE;

		int submitOrder;

		void addTexture(Texture t) {
			T.textureVec.push_back(t);
		}

		void render(float x, float y, float sx, float sy, UIOriginH RegH, UIOriginV RegV, bool isVisible) {
			this->x = x;
			this->y = y;
			this->sx = sx;
			this->sy = sy;
			this->RegH = RegH;
			this->RegV = RegV;
			this->isVisible = isVisible;
		}

		/**
		 * Returns the pixel coordinates of this UIElement:
		 * - coordinates[0] is the upper left corner;
		 * - coordinates[1] is the bottom right corner.
		 */
		std::array<glm::vec2, 2> getPixelCoordinates(int screenW, int screenH) {
			return ::getPixelCoordinates(screenW,screenH, x, y, sx, sy, RegH, RegV, T.width, T.height);
		}

		/**
		 * Recreates the mesh of this UIElement
		 */
		void createMesh(VertexDescriptor* VD, int screenW, int screenH, BaseProject* BP) {
			int mainStride = sizeof(UIVertex);
			// std::cout << UI_DEBUG_STRING << " mainStride: " << mainStride << std::endl;
			this->M = new Model();
			this->M->indices.resize(6);
			this->M->vertices.resize(4 * mainStride);
			
			UIVertex *V_vertex = (UIVertex *)(&this->M->vertices[0]);
			std::array<glm::vec2, 2> coordinates = getPixelCoordinates(screenW, screenH);
			
			makeUIVertex(V_vertex, coordinates[0].x, coordinates[0].y, 0, 0, screenW, screenH);
			V_vertex++;

			makeUIVertex(V_vertex, coordinates[1].x, coordinates[0].y, 1, 0, screenW, screenH);
			V_vertex++;
			
			makeUIVertex(V_vertex, coordinates[0].x, coordinates[1].y, 0, 1, screenW, screenH);
			V_vertex++;

			makeUIVertex(V_vertex, coordinates[1].x, coordinates[1].y, 1, 1, screenW, screenH);
			V_vertex++;
			
			this->M->indices[0] = 0;
			this->M->indices[1] = 1;
			this->M->indices[2] = 2;
			this->M->indices[3] = 1;
			this->M->indices[4] = 2;
			this->M->indices[5] = 3;

			// std::cout << UI_DEBUG_STRING << " init mesh" << std::endl;
			this->M->initMesh(BP, VD, false);
		}

		void makeUIVertex(UIVertex *V, float px, float py, float tx, float ty, int screenW, int screenH) {
			// std::cout << UI_DEBUG_STRING << " makeUIVertex: ";
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
		 * Recreates the local descript set using textureVec[textureId] as the texture
		 * Throws an error if textureId is an invalid index of the vector
		 */
		void recreateDescriptorSet(DescriptorSetLayout *DSL, BaseProject *BP, int textureId) {
			if (textureId >= this->T.textureVec.size() || textureId < 0) {
				error("Tried to access texture out of bounds: " + std::to_string(textureId) + " | texture vector size " + std::to_string(this->T.textureVec.size()));
			}

			this->DS.init(BP, DSL, {this->T.textureVec[textureId].getViewAndSampler()});
		}

		/**
		 * Manages the scale of this UIElement based on its resize type and the screen dimensions
		 * NOTE: scalable elements use DEFAULT_WINDOW_WIDTH and DEFAULT_WINDOW_HEIGHT as the "default" scaling
		 */
		void scaleToScreen(int screenW, int screenH) {
			float aspectRatio = this->sx / this->sy, xRatio = (float)screenW / DEFAULT_WINDOW_WIDTH, yRatio = (float)screenH / DEFAULT_WINDOW_HEIGHT;

			if (this->resize == KEEP_ASPECT_RATIO) {
				if (xRatio < yRatio) {
					this->sx = xRatio;
					this->sy = xRatio / aspectRatio;
				} else {
					this->sx = yRatio * aspectRatio;
					this->sy = yRatio;
				}
			} else {
				if (this->resize == FULL_RESIZABLE || this->resize == WIDTH_ONLY_RESIZABLE)
					this->sx = xRatio;
				
				if (this->resize == FULL_RESIZABLE || this->resize == HEIGHT_ONLY_RESIZABLE)
					this->sy = yRatio;
			}
		}

		/**
		 * Returns true if the point (_x, _y) is within the hitbox of this UIElement
		 */
		bool isPointInsideHitbox(float _x, float _y, int screenW, int screenH) {
			std::array<glm::vec2, 2> coordinates = getPixelCoordinates(screenW, screenH);
			float x1 = coordinates[0].x, x2 = coordinates[1].x;
			float y1 = coordinates[0].y, y2 = coordinates[1].y;

			return isPointInsideRectangle(_x, _y, x1, y1, x2, y2);
		}
	};

	/**
	 * Stores button specific data
	 */
	struct ButtonData {
		int id;
		bool hovered;
		bool clicked;
	};

	/**
	 * Stores slider specific data (the id is the index of the map)
	 */
	struct SliderData {
		bool moving;
		float currentMaxScale;
		glm::vec2 upperLeftCorner;
		glm::vec2 lowerRightCorner;
	};

	enum VOLUME_STATUS {VOLUME_LOW, VOLUME_MEDIUM, VOLUME_HIGH};

	VOLUME_STATUS volume_status = VOLUME_HIGH;

	VertexDescriptor UI_VD;
	BaseProject* BP;
	DescriptorSetLayout UI_DSL;
	RenderPass UI_RP;

	int screenW, screenH;	//should be swapchain width and height instead of screen, but can't access it (TextMaker has the same bug)
	
	double mousePosX, mousePosY;
	
	std::unordered_map<int, UIElement> UIElementsMap = {};
	std::list<ButtonData> ButtonsList = {};
	std::unordered_map<int, SliderData> SlidersMap = {};
	
	bool commandBufferMustUpdate = false;

	/**
	 * Takes care of the common parts of initElement(<TextureFilesWithParams>) and initElement(<TextureDataWithParams>)
	 * Assumes the given id is not used by another UIElement
	 */
	void initElement(int id, bool isTransparent, ResizableType resize, UIElementType type) {
		UIElementsMap[id].P.init(BP, &UI_VD, "shaders/UIElement.vert.spv", "shaders/UIElement.frag.spv", {&UI_DSL});
		UIElementsMap[id].P.setCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
		UIElementsMap[id].P.setCullMode(VK_CULL_MODE_NONE);
		UIElementsMap[id].P.setTransparency(isTransparent);

		UIElementsMap[id].submitOrder = DEFAULT_SUBMIT_ORDER + id;
		UIElementsMap[id].resize = resize;
		UIElementsMap[id].type = type;

		switch(type) {
		case UI_BUTTON:
			if (UIElementsMap[id].T.textureVec.size() < 3) {
				error("button " + std::to_string(id) + "has less than 3 textures\nButtons need a minimum of 3 textures to work");
			}

			ButtonsList.push_back({id, false, false});
			break;
		case UI_SLIDER:
			SlidersMap[id] = {false};
			break;
		default:
			break;
		}
	}

	/**
	 * Initializes the slider hitbox, using DEFAULT_WINDOW_* as the window dimension
	 */
	void setSliderHitbox(int id) {
		std::array<glm::vec2, 2> coordinates = UIElementsMap[id].getPixelCoordinates(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
		SlidersMap[id].currentMaxScale = 1.0f;
		SlidersMap[id].upperLeftCorner = coordinates[0];
		SlidersMap[id].lowerRightCorner = coordinates[1];
	}

	/**
	 * Updates the slider hitbox in case of window resize
	 */
	void updateSliderHitbox(int id, int sW, int sH) {
		UIElement temp = UIElementsMap[id];

		// sy is passed both as sx and sy, since sx is used as the value of the slider
		std::array<glm::vec2, 2> coordinates = ::getPixelCoordinates(sW, sH, temp.x, temp.y, temp.sy, temp.sy, temp.RegH, temp.RegV, temp.T.width, temp.T.height);
		SlidersMap[id].currentMaxScale = temp.sy;
		SlidersMap[id].upperLeftCorner = coordinates[0];
		SlidersMap[id].lowerRightCorner = coordinates[1];
	}

public:
	UIMaker(BaseProject *_BP) {
		BP = _BP;
	}
	
	~UIMaker() = default;

	//--------------------------------------------

	/**
	 * Initialises a UIElement with the given id and parameters
	 * Prints a warning if the id is already in use and skips execution
	 * NOTE: the element is initialized as not visible, you need a call to renderUI with proper parameters to see it
	 */
	void initElement(int id, TextureFilesWithParams textureFile) {
		if (UIElementsMap.find(id) != UIElementsMap.end()) {
			warning("Another element with the same id ()" + std::to_string(id) + ") is already present in UIElementsMap, skipping insertion");
			return;
		}

		int garbage;

		for (auto t : textureFile.TextureFiles) {
			Texture temp;
			temp.init(BP, t);

			UIElementsMap[id].addTexture(temp);
		}

		/// Assumes all texture with the same size
		unsigned char* pixels = stbi_load(textureFile.TextureFiles.front().c_str(), &UIElementsMap[id].T.width, &UIElementsMap[id].T.height, &garbage, STBI_rgb_alpha);
		stbi_image_free(pixels);

		initElement(id, textureFile.isTransparent, textureFile.resize, textureFile.type);
	}

	/**
	 * Initialises a UIElement with the given id and parameters
	 * Prints a warning if the id is already in use and skips execution
	 * NOTE: the element is initialized as not visible, you need a call to renderUI with proper parameters to see it
	 */
	void initElement(int id, TextureDataWithParams textureData) {
		if (UIElementsMap.find(id) != UIElementsMap.end()) {
			warning("Another element with the same id ()" + std::to_string(id) + ") is already present in UIElementsMap, skipping insertion");
			return;
		}

		for (auto t : textureData.TextureData) {
			Texture temp;
			temp.initPixels(BP, t.width, t.height, 4, sizeof(uint8_t), {t.pixels.data()});

			UIElementsMap[id].addTexture(temp);
		}

		/// Assumes all texture with the same size
		UIElementsMap[id].T.width = textureData.TextureData.front().width;
		UIElementsMap[id].T.height = textureData.TextureData.front().height;

		initElement(id, textureData.isTransparent, textureData.resize, textureData.type);
	}

	/**
	* Finalizes the UIMaker initialization, setting global parameters and (eventually) adding a UIElement for all given textures
	* To be called after all other initElements
	* NOTE: the id of the UIElement depends on the position in the lists TextureFiles and TextureDataList (in order)
	* NOTE: the elements are initialized as not visible, you need a call to renderUI with proper parameters to see them
	*/
	void init(int sW, int sH, std::list<TextureFilesWithParams> TextureFilesList = {}, std::list<TextureDataWithParams> TextureDataList = {}, int so = DEFAULT_SUBMIT_ORDER)  {
		// std::cout << UI_DEBUG_STRING << " UI init" << std::endl;
		screenW = sW;
		screenH = sH;

		UI_RP.init(BP, sW, sH, -1, RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, BP));
		UI_RP.properties[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		UI_RP.properties[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		UI_RP.properties[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		int i = 0;
		for (auto textureChunk : TextureFilesList) {
			// if there's already a UIElement with that id, try the next one
			while (UIElementsMap.find(i) != UIElementsMap.end()) {
				i++;
			}

			initElement(i, textureChunk);
			i++;
		}

		for (auto textureChunk : TextureDataList) {
			while (UIElementsMap.find(i) != UIElementsMap.end()) {
				i++;
			}

			initElement(i, textureChunk);
			i++;
		}

		/// Sets up the Vertex Descriptor and Descriptor Set Layout
		// std::cout << UI_DEBUG_STRING << " Create UI descriptor sets and vertex layouts" << std::endl;
		UI_VD.init(BP, {{0, sizeof(UIVertex), VK_VERTEX_INPUT_RATE_VERTEX}}, {
			{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, pos), sizeof(glm::vec2), POS2D},
			{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, uv), sizeof(glm::vec2), UV}
		});

		UI_DSL.init(BP, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}});

		BP->DPSZs.texturesInPool += UIElementsMap.size();
		BP->DPSZs.setsInPool += UIElementsMap.size();
	}

	void pipelinesAndDescriptorSetsInit() {
		// std::cout << UI_DEBUG_STRING << " UI pipelines and descriptor sets init" << std::endl;
		UI_RP.create();

		for (auto& e : UIElementsMap) {
			// std::cout << UI_DEBUG_STRING << " creating pipeline #" << e.first << std::endl;
			e.second.P.create(&UI_RP);
			//TODO to create >1 viewports, need multiViewport feature; is it worth it?
		}

		/// Creates a descriptor set for each UIElement
		// std::cout << UI_DEBUG_STRING << " UI descritor sets init";
		for (auto& e : UIElementsMap) {
			e.second.createDescriptorSet(&UI_DSL, BP);
		}
	}

	static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
		// std::cout << UI_DEBUG_STRING << " populate command buffer access" << std::endl;
		UIMaker *T = ((UIMakerAndModel *)Params)->ui;
		T->populateCommandBuffer(commandBuffer, currentImage);
	}

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// std::cout << UI_DEBUG_STRING << " populate command buffer" << std::endl;
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

	//--------------------------------------------

	std::list<InteractedUIElementData> updateMouseStatus(double x, double y, bool mouseClick = false, bool holding = false) {
		std::list<InteractedUIElementData> ret = {};
		this->mousePosX = x;
		this->mousePosY = y;
		// std::cout << UI_DEBUG_STRING << " updating mouse status: " << x << " x " << y << " | mouseClick = " << mouseClick << " | holding = " << holding << std::endl;

		// After the cursor moves, checks every button to see if their status changed
		for (auto &b : ButtonsList) {
			bool inside = UIElementsMap[b.id].isPointInsideHitbox((float)mousePosX, (float)mousePosY, screenW, screenH);
			// std::cout << UI_DEBUG_STRING << " " << b.id << " hovered: " << b.hovered << "; inside: " << inside << std::endl;

			if (b.hovered) {
				if (!inside) {
					// if the button was previously hovered and now the cursor is outside the hitbox
					UIElementsMap[b.id].recreateDescriptorSet(&UI_DSL, BP, 0);
					b.hovered = false;
					commandBufferMustUpdate = true;
				} else {
					if (mouseClick && !b.clicked) {
						// if the button was previously hovered and now the cursor clicks it
						// std::cout << UI_DEBUG_STRING << " clicked a button" << std::endl;
						b.clicked = true;
						UIElementsMap[b.id].recreateDescriptorSet(&UI_DSL, BP, 2);
						commandBufferMustUpdate = true;
						ret.push_back({b.id});
					} else if (b.clicked && !mouseClick) {
						// if the button was previously clicked, restore its texture to hovered
						b.clicked = false;
						UIElementsMap[b.id].recreateDescriptorSet(&UI_DSL, BP, 1);
						commandBufferMustUpdate = true;
					}
				}
			} else if (!b.hovered && inside) {
				// if the button was not previously hovered and now the button is inside the hitbox
				UIElementsMap[b.id].recreateDescriptorSet(&UI_DSL, BP, 1);
				b.hovered = true;
				commandBufferMustUpdate = true;
			}
		}

		for (auto &s : SlidersMap) {
			// std::cout << UI_DEBUG_STRING << " slider #" << s.first << ": moving = " << s.second.moving << " | upper left corner = " << s.second.upperLeftCorner.x << " x " << s.second.upperLeftCorner.y << " | lower right corner = " << s.second.lowerRightCorner.x << " x " << s.second.lowerRightCorner.y << " | xscale = " << UIElementsMap[s.first].sx << std::endl;
			if (holding && s.second.moving) {
				// the user is moving the slider around (doesn't matter if the cursor is on the slider, as long as they keep pressing it)
				if (x <= s.second.upperLeftCorner.x) {
					// lower bound
					if (UIElementsMap[s.first].sx != 0.0f) {
						UIElementsMap[s.first].sx = 0.0f;
						commandBufferMustUpdate = true;
						ret.push_back({s.first, 0.0f});
					}
				} else if (x >= s.second.lowerRightCorner.x) {
					// upper bound
					if (UIElementsMap[s.first].sx != s.second.currentMaxScale) {
						UIElementsMap[s.first].sx = s.second.currentMaxScale;
						commandBufferMustUpdate = true;
						ret.push_back({s.first, 1.0f});
					}
				} else {
					// value in between
					UIElementsMap[s.first].sx = (x - s.second.upperLeftCorner.x)/(s.second.lowerRightCorner.x - s.second.upperLeftCorner.x) * s.second.currentMaxScale;
					commandBufferMustUpdate = true;
					ret.push_back({s.first, UIElementsMap[s.first].sx / s.second.currentMaxScale});
				}
			} else if (mouseClick && isPointInsideRectangle(x, y, s.second.upperLeftCorner, s.second.lowerRightCorner)) {
				// the user clicks inside the "hitbox" of the slider, setting the slider to that point and saving that the slider is moving
				UIElementsMap[s.first].sx = (x - s.second.upperLeftCorner.x)/(s.second.lowerRightCorner.x - s.second.upperLeftCorner.x) * s.second.currentMaxScale;
				s.second.moving = true;
				commandBufferMustUpdate = true;
				ret.push_back({s.first, UIElementsMap[s.first].sx / s.second.currentMaxScale});
			} else if (s.second.moving) {
				s.second.moving = false;
			}

			// if the modified slider was the volume, check to see if the plaque needs updating
			if (s.first == UI_ID_SLIDER_VOLUME && commandBufferMustUpdate) {
				VOLUME_STATUS old_volume_status = volume_status;

				if (UIElementsMap[s.first].sx == 0.0f) {
					volume_status = VOLUME_LOW;
				} else if (UIElementsMap[s.first].sx == s.second.currentMaxScale) {
					volume_status = VOLUME_HIGH;
				} else {
					volume_status = VOLUME_MEDIUM;
				}

				if (old_volume_status != volume_status) {
					switch(volume_status) {
					case VOLUME_LOW:
						UIElementsMap[UI_ID_SLIDER_VOLUME_PLAQUE].recreateDescriptorSet(&UI_DSL, BP, 2);
						break;
					case VOLUME_MEDIUM:
						UIElementsMap[UI_ID_SLIDER_VOLUME_PLAQUE].recreateDescriptorSet(&UI_DSL, BP, 1);
						break;
					case VOLUME_HIGH:
						UIElementsMap[UI_ID_SLIDER_VOLUME_PLAQUE].recreateDescriptorSet(&UI_DSL, BP, 0);
						break;
					}
				}
			}
		}

		return ret;
	}

	void toggleVisibility(int id) {
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		elem->second.isVisible = !elem->second.isVisible;
		commandBufferMustUpdate = true;
	}

	/**
	* Notifies that the UI element with the given id needs to be updated
	* Throws an error if the id isn't present in UIElementsMap, and a warning if either sx or sy are 0
	*/
	void renderUI(float x, float y, int id, UIOriginH RegH = UIO_LEFT, UIOriginV RegV = UIO_TOP, float sx = 1.0f, float sy = 1.0f, bool isVisible = true) {
		// std::cout << UI_DEBUG_STRING << " renderUI id = " << id << std::endl;
		if (sx == 0 || sy == 0)
			warning("1-dimensional UI element: id = " + std::to_string(id) + ", sx = " + std::to_string(sx) + ", sy = " + std::to_string(sy));
		
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		UIElementsMap[id].render(x, y, sx, sy, RegH, RegV, isVisible);
		commandBufferMustUpdate = true;

		if (UIElementsMap[id].type == UI_SLIDER) {
			setSliderHitbox(id);
		}
	}

	/**
	* Rearranges UI elements on screen resize
	*/
	void resizeScreen(int sW, int sH) {
		// std::cout << UI_DEBUG_STRING << " UI resizeScreen" << std::endl;
		screenW = sW;
		screenH = sH;
		UI_RP.width = sW;
		UI_RP.height = sH;

		for (auto &e : UIElementsMap) {
			if (e.second.resize != NOT_RESIZABLE) {
				e.second.scaleToScreen(sW, sH);
				if (e.second.type == UI_SLIDER) {
					updateSliderHitbox(e.first, sW, sH);
				}
			}
		}

		commandBufferMustUpdate = true;
	}

	/**
	 * If at least a UIElement was changed (commandBufferMustUpdate = true), redraws every UIElement.
	 * Also, updates mousePos[X, Y] if the cursor isn't locked
	 */
	void updateCommandBuffer() {
		// std::cout << UI_DEBUG_STRING << " UI update command buffer" << std::endl;
		// debugPrint();

		if (commandBufferMustUpdate) {
			//could add an update field to each UIElement to avoid updating unchanged elements, but doesn't seem worth it
			for (auto& e : UIElementsMap) {
				createUIMesh(e.first);	// creates the new mesh
				
				UIMakerAndModel *uim = (UIMakerAndModel *)malloc(sizeof(UIMakerAndModel));
				uim->ui = this;
				uim->M = e.second.M;
				BP->submitCommandBuffer("ui" + std::to_string(e.first), e.second.submitOrder, UIMaker::populateCommandBufferAccess, uim, UIMaker::freeCommandBuffer);
			}

			commandBufferMustUpdate = false;
		}
	}

	/**
	* Creates the mesh for the given UI element
	*/
	void createUIMesh(int id) {
		// std::cout << UI_DEBUG_STRING << " createUImesh with id " << id << std::endl;
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));
		
		// std::cout << UI_DEBUG_STRING << " creating mesh of " << id << std::endl;
		UIElementsMap[id].createMesh(&UI_VD, screenW, screenH, BP);
	}

	void recreateUIDescriptorSet(int id, int idTexture) {
		// std::cout << UI_DEBUG_STRING << " re-create DS with id " << id << "and texture " << idTexture << std::endl;
		auto elem = UIElementsMap.find(id);
		if (elem == UIElementsMap.end())
			error("Invalid UI id: " + std::to_string(id));

		UIElementsMap[id].recreateDescriptorSet(&UI_DSL, BP, idTexture);
	}

	void renderMainMenu() {
		renderUI(0.0f, 0.0f, UI_ID_MENU_BACKGROUND, UIO_CENTER, UIO_MIDDLE);

		renderUI(0.0f, -1.0f, UI_ID_TITLE, UIO_CENTER, UIO_TOP);

		renderUI(0.0f, -0.085f, UI_ID_BUTTON_START, UIO_CENTER, UIO_MIDDLE);
		renderUI(0.0f, 0.085f, UI_ID_BUTTON_QUIT, UIO_CENTER, UIO_MIDDLE);
	}

	void toggleMainMenu() {
		toggleVisibility(UI_ID_TITLE);
		toggleVisibility(UI_ID_BUTTON_START);
	}

	void renderPauseMenu() {
		renderUI(0.0f, 0.0f, UI_ID_MENU_BACKGROUND, UIO_CENTER, UIO_MIDDLE);

		renderUI(0.0f, -0.085f, UI_ID_BUTTON_RESUME, UIO_CENTER, UIO_MIDDLE);
		renderUI(0.0f, 0.085f, UI_ID_BUTTON_QUIT, UIO_CENTER, UIO_MIDDLE);

		renderUI(-1.0f, 1.0f, UI_ID_BUTTON_SCENE1, UIO_LEFT, UIO_BOTTOM);
		renderUI(-0.85f, 1.0f, UI_ID_BUTTON_SCENE2, UIO_LEFT, UIO_BOTTOM);

		renderUI(-1.0f, -0.06, UI_ID_SLIDER_VOLUME, UIO_LEFT, UIO_MIDDLE);
		renderUI(-1.0f, -0.06, UI_ID_SLIDER_VOLUME_BACKGROUND, UIO_LEFT, UIO_MIDDLE);
		renderUI(-1.0f, -0.06, UI_ID_SLIDER_VOLUME_PLAQUE, UIO_LEFT, UIO_BOTTOM);
		
		renderUI(-1.0f, 0.06, UI_ID_SLIDER_SENSITIVITY, UIO_LEFT, UIO_MIDDLE);
		renderUI(-1.0f, 0.06, UI_ID_SLIDER_SENSITIVITY_BACKGROUND, UIO_LEFT, UIO_MIDDLE);
		renderUI(-1.0f, 0.06, UI_ID_SLIDER_SENSITIVITY_PLAQUE, UIO_LEFT, UIO_TOP);

		renderUI(1.0f, -1.0f, UI_ID_COMMANDS, UIO_RIGHT, UIO_TOP, 1.25f, 1.25f);
	}

	/** 
	* Toggles visibility of the pause menu
	*/
	void togglePauseMenu() {
		toggleVisibility(UI_ID_MENU_BACKGROUND);
		toggleVisibility(UI_ID_BUTTON_RESUME);
		toggleVisibility(UI_ID_BUTTON_QUIT);
		toggleVisibility(UI_ID_BUTTON_SCENE1);
		toggleVisibility(UI_ID_BUTTON_SCENE2);
		toggleVisibility(UI_ID_SLIDER_VOLUME);
		toggleVisibility(UI_ID_SLIDER_VOLUME_BACKGROUND);
		toggleVisibility(UI_ID_SLIDER_VOLUME_PLAQUE);
		toggleVisibility(UI_ID_SLIDER_SENSITIVITY);
		toggleVisibility(UI_ID_SLIDER_SENSITIVITY_BACKGROUND);
		toggleVisibility(UI_ID_SLIDER_SENSITIVITY_PLAQUE);
		toggleVisibility(UI_ID_COMMANDS);
	}

	//--------------------------------------------

	/**
	* Removes a single UI element, given its id
	*/
	void removeUIElement(int id) {
		// std::cout << UI_DEBUG_STRING << " removeUIElement id = " << id << std::endl;
		//TODO implement proper deconstructor
		UIElementsMap.erase(id);
		commandBufferMustUpdate = true;
	}

	/**
	* Removes all UI elements
	*/
	void removeUI() {
		// std::cout << UI_DEBUG_STRING << " removeUI" << std::endl;
		//TODO implement proper deconstructor
		UIElementsMap.clear();
		commandBufferMustUpdate = true;
	}

	void pipelinesAndDescriptorSetsCleanup() {
		// std::cout << UI_DEBUG_STRING << " UI pipelines and descript sets cleanup" << std::endl;
		// std::cout << UI_DEBUG_STRING << "\tCleaning pipelines" << std::endl;
		for (auto& e : UIElementsMap)
			e.second.P.cleanup();
	
		// std::cout << UI_DEBUG_STRING << "\tCleaning render pass" << std::endl;
		UI_RP.cleanup();
		
		// std::cout << UI_DEBUG_STRING << "\tCleaning descriptor sets" << std::endl;
		for (auto& e : UIElementsMap)
			e.second.DS.cleanup();
	}

	void localCleanup() {
		// std::cout << UI_DEBUG_STRING << " local cleanup" << std::endl;
		// std::cout << UI_DEBUG_STRING << "\tCleaning textures" << std::endl;
		for (auto& e : UIElementsMap) 
			for (auto& t : e.second.T.textureVec)
				t.cleanup();
		
		// std::cout << UI_DEBUG_STRING << "\tCleaning models" << std::endl;
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

		// std::cout << UI_DEBUG_STRING << "\tCleaning descriptor set layout" << std::endl;
		UI_DSL.cleanup();
		
		// std::cout << UI_DEBUG_STRING << "\tDestroying pipelines" << std::endl;
		for (auto& e : UIElementsMap) 
			e.second.P.destroy();

		// std::cout << UI_DEBUG_STRING << "\tDestroying render pass" << std::endl;
		UI_RP.destroy();
	}

	static void freeCommandBuffer(void *Params) {
		// std::cout << UI_DEBUG_STRING << " free command buffer" << std::endl;
		Model *M = ((UIMakerAndModel *)Params)->M;
		M->cleanup();
		delete(M);
		
		free(Params);
	}

	void deleteMainMenu() {
		removeUIElement(UI_ID_TITLE);
		removeUIElement(UI_ID_BUTTON_START);
	}

	//--------------------------------------------

	void debugPrint() {
		std::cout << UI_DEBUG_STRING << " DEBUG PRINTING -------------------------------" << std::endl;

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

			std::cout << "\tResizable type: ";
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
			case KEEP_ASPECT_RATIO:
				std::cout << "keep aspect ratio";
				break;
			}
			std::cout << std::endl;

			std::cout << "\tUI element type: ";
			switch (e.second.type) {
			case UI_NORMAL:
				std::cout << "normal";
				break;
			case UI_BUTTON:
				std::cout << "button";
				break;
			case UI_SLIDER:
				std::cout << "slider";
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

			std::cout << "\tIsVisible: " << e.second.isVisible << std::endl;
		}

		std::cout << "Buttons list size: " << ButtonsList.size() << std::endl;
		for (auto b : ButtonsList) {
			std::cout << "[" << b.id << "]" << std::endl;

			std::cout << "\tClicked:" << b.clicked << std::endl;
			std::cout << "\tHovered:" << b.hovered << std::endl;
		}

		std::cout << "Sliders list size: " << SlidersMap.size() << std::endl;
		for (auto s : SlidersMap) {
			std::cout << "[" << s.first << "]" << std::endl;

			std::cout << "\tMoving:" << s.second.moving << std::endl;
			std::cout << "\tUpper left corner: " << s.second.upperLeftCorner.x << " x " << s.second.upperLeftCorner.y << std::endl;
			std::cout << "\tLower right corner: " << s.second.lowerRightCorner.x << " x " << s.second.lowerRightCorner.y << std::endl;
		}

		std::cout << UI_DEBUG_STRING << " END DEBUG PRINTING ---------------------------" << std::endl;
	}
};

#endif
