#include <list>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include "Texture.hpp"
// #define  STARTER_IMPLEMENTATION
// #include "modules/Starter.hpp"

#define DEFAULT_SUBMIT_ORDER 9999

enum UIOriginH {UIO_LEFT, UIO_CENTER, UIO_RIGHT};
enum UIOriginV {UIO_TOP, UIO_MIDDLE, UIO_BOTTOM};

struct UIElement {
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
	 * */
	UIOriginH RegH;
	UIOriginV RegV;
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
	Texture t;
};

struct UIMaker {
	VertexDescriptor VD;	
	
	BaseProject *BP;
	int screenW, screenH;
	int submitOrder;

	DescriptorSetLayout DSL;
	RenderPass RP;
	Pipeline P;	//TODO: UI elements with scissor need their own pipeline
	std::unordered_map<int, Model*> ModelsMap = {};
	std::unordered_map<int, UITextureData> TexturesMap = {};
	std::unordered_map<int, DescriptorSet> DSMap = {};
	
	std::unordered_map<int, UIElement> UIElementsMap = {};
	int maxUIid = 0;
	
	//TODO implement this + does it work with removeUIElement?
	//std::unordered_map<int, bool> UpdateMap = {};
	bool commandBufferMustUpdate = false;	//if true, at least one UI element needs updating
	bool updateAll = false;					//if true, all UI elements need updating
	
	//--------------------------------------------
	void init(BaseProject *_BP, int sW, int sH, std::list<std::string> TextureFiles = {}, std::list<ProceduralTextures::TextureData> TextureDataList = {}, int so = DEFAULT_SUBMIT_ORDER);
	void createUIDescriptorSetAndVertexLayout();
 	void createUIPipeline();
	void createUIDescriptorSets();
	void pipelinesAndDescriptorSetsInit();
	static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);
	
	int renderUI(float x, float y, int id = -1, UIOriginH RegH = UIO_LEFT, UIOriginV RegV = UIO_TOP, float sx = 1.0f, float sy = 1.0f);
	void resizeScreen(int sW, int sH);
	void updateCommandBuffer();
	void createUIMesh(int id);

	void removeUIElement(int id);
	void removeUI();
	void pipelinesAndDescriptorSetsCleanup();
	void localCleanup();
	static void freeCommandBuffer(void *Params);

	void makeUIVertex(UIVertex *V, int px, int py, int tx, int ty);
	void pixelToScr(float x, float y, float &sx, float &sy);
	void atlasToUV(int x, int y, float &u, float &v);

	void debugPrint();
};

struct UIMakerAndModel {
	UIMaker *ui;
	Model *M;
};

//--------------------------------------------
inline void UIMaker::debugPrint() {
	std::cout << "[UI DEBUG] DEBUG PRINTING -------------------------------" << std::endl;
	std::cout << "Models map\n\t size: " << ModelsMap.size() << std::endl;
	for	(auto M : ModelsMap) {
		std::cout << "\t" << M.first << ":";
		for (auto i : M.second->indices) {
			std::cout << " " << i;
		}
		std::cout << std::endl;
	}
	std::cout << "Textures map\n\t size: " << TexturesMap.size() << std::endl;
	for (auto T : TexturesMap) {
		std::cout << "\t" << T.first << ": " << T.second.width << " x " << T.second.height << std::endl;
	}
	std::cout << "DS maps map\n\t size: " << DSMap.size() << std::endl;
	std::cout << "UI elements map\n\t size: " << UIElementsMap.size() << std::endl;
	for (auto E : UIElementsMap) {
		std::cout << "\t" << E.first << ": (" << E.second.x << "," << E.second.y << ")" << std::endl;
	}
	std::cout << "[UI DEBUG] END DEBUG PRINTING ---------------------------" << std::endl;
}

/**
 * Initializes the UIMaker with the given textures
 * If not passed, TextureFiles and TextureDataList are empty lists, and so = DEFAULT_SUBMIT_ORDER
 * */
inline void UIMaker::init(BaseProject *_BP, int sW, int sH, std::list<std::string> TextureFiles, std::list<ProceduralTextures::TextureData> TextureDataList, int so) {
	/// Initial init
	BP = _BP;
	screenW = sW;
	screenH = sH;
	submitOrder = so;

	createUIDescriptorSetAndVertexLayout();
	createUIPipeline();

	RP.init(BP, sW, sH, -1, RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, BP));
	RP.properties[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	RP.properties[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	RP.properties[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	RP.properties[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	/// Texture loading
	//TODO add the ability to pass array of textures for UI elements that change texture under certain conditions (buttons, switches)
	int i = 0;
	for (auto t : TextureFiles) {
		Texture temp;
		temp.init(BP, t);
		TexturesMap[i].t = temp;
		//TODO find a way to get height and width when loading from file

		i++;
	}

	for (auto t : TextureDataList) {
		Texture temp;
		temp.initPixels(BP, t.width, t.height, 4, sizeof(uint8_t), {t.pixels.data()});
		TexturesMap[i].t = temp;
		TexturesMap[i].width = t.width;
		TexturesMap[i].height = t.height;

		i++;
	}
	
	BP->DPSZs.texturesInPool += TexturesMap.size();
	BP->DPSZs.setsInPool += TexturesMap.size();
}

/**
 * Sets up the Vertex Descriptor and Descriptor Set Layout
 * */
inline void UIMaker::createUIDescriptorSetAndVertexLayout() {
	VD.init(BP, {{0, sizeof(UIVertex), }}, {
		{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, pos), sizeof(glm::vec2), POSITION},
		{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(UIVertex, uv), sizeof(glm::vec2), UV}
	});

	DSL.init(BP, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}});
}

/**
 * Sets up the main Pipeline with a default shader
 * */
inline void UIMaker::createUIPipeline() {
	P.init(BP, &VD, "shaders/UIElement.vert.spv", "shaders/UIElement.frag.spv", {&DSL});
	P.setCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
	P.setCullMode(VK_CULL_MODE_NONE);
	P.setTransparency(false);
}

/**
 * Notifies that the UI element with the given id needs to be updated
 * */
inline int UIMaker::renderUI(float x, float y, int id, UIOriginH RegH, UIOriginV RegV, float sx, float sy) {
	std::cout << "[UI DEBUG] renderUI id = " << id << std::endl;
    if (id == -1) {
		id = maxUIid;
		maxUIid++;
	}

	if (id > maxUIid) {
		maxUIid = id;
	}

	UIElementsMap[id] = {x, y, sx, sy, RegH, RegV};
    commandBufferMustUpdate = true;

	return id;
}

/**
 * Removes a single UI element, given its id
 * */
inline void UIMaker::removeUIElement(int id) {
	UIElementsMap.erase(id);
	commandBufferMustUpdate = true;
}

/**
 * Removes all UI elements
 * */
inline void UIMaker::removeUI() {
	UIElementsMap.clear();
	commandBufferMustUpdate = true;
}

/**
 * Rearranges UI elements on screen resize
 * */
inline void UIMaker::resizeScreen(int sW, int sH) {
	std::cout << "[UI DEBUG] UI resizeScreen" << std::endl;
	screenW = sW;
	screenH = sH;
	RP.width = sW;
	RP.height = sH;
	commandBufferMustUpdate = true;
}

/**
 * Transforms pixel to screen coordinates
 * */
inline void UIMaker::pixelToScr(float x, float y, float &sx, float &sy) {
	sx = (x + 0.5f) / (float)screenW * 2.0f - 1.0f;
	sy = (y + 0.5f) / (float)screenH * 2.0f - 1.0f;
}

inline void UIMaker::atlasToUV(int x, int y, float &u, float &v) {
	u = (float)x;
	v = (float)y;
}

inline void UIMaker::makeUIVertex(UIVertex *V, int px, int py, int tx, int ty) {
	pixelToScr(px, py, V->pos.x, V->pos.y);
	atlasToUV(tx, ty, V->uv.x, V->uv.y);
	std::cout << "[UI DEBUG] makeUIVertex: pos = (" << V->pos.x << "," << V->pos.y << ") | uv = (" << V->uv.x << "," << V->uv.y << ")" << std::endl;
}

/**
 * Creates the mesh for the given UI element
 * */
inline void UIMaker::createUIMesh(int id) {
	std::cout << "[UI DEBUG] createUImesh with id " << id << std::endl;
	ModelsMap[id] = new Model();
	
	int mainStride = VD.Bindings[0].stride;
	
	//TODO not entirely sure what indices and stride are supposed to be
	ModelsMap[id]->indices.resize(6);
	ModelsMap[id]->vertices.resize(4 * mainStride);
	std::cout << "[UI DEBUG] mainStride: " << mainStride << std::endl;

	float tpx = 0;
	float tpy = 0;
	
	UIVertex *V_vertex = (UIVertex *)(&ModelsMap[id]->vertices[0]);
	auto& Blk = UIElementsMap[id];

	tpx = (Blk.x + 1.0f)/2.0f * screenW - Blk.sx * (
		(Blk.RegH == UIO_RIGHT  ? (float)TexturesMap[id].width		: 0.0f) +
		(Blk.RegH == UIO_CENTER ? (float)TexturesMap[id].width/2.0f : 0.0f)
	);
	tpy = (Blk.y + 1.0f)/2.0f * screenH - Blk.sy * (
		(Blk.RegV == UIO_BOTTOM ? (float)TexturesMap[id].height		: 0.0f) +
		(Blk.RegV == UIO_MIDDLE ? (float)TexturesMap[id].height/2.0f: 0.0f)
	);
	
	//TODO not sure if vertex data is correct
	// makeUIVertex(V_vertex, tpx + Blk.sx, tpy + Blk.sy, Blk.x, Blk.y);
	makeUIVertex(V_vertex, tpx + Blk.sx, tpy + Blk.sy, 0, 0);
	V_vertex++;

	// makeUIVertex(V_vertex, tpx + (float)(TexturesMap[id].width) * Blk.sx, tpy + Blk.sy, Blk.x + TexturesMap[id].width, Blk.y);
	makeUIVertex(V_vertex, tpx + (float)(TexturesMap[id].width) * Blk.sx, tpy + Blk.sy, 1, 0);
	V_vertex++;
	
	// makeUIVertex(V_vertex, tpx + Blk.sx, tpy + (float)(TexturesMap[id].height) * Blk.sy, Blk.x, Blk.y + TexturesMap[id].height);
	makeUIVertex(V_vertex, tpx + Blk.sx, tpy + (float)(TexturesMap[id].height) * Blk.sy, 0, 1);
	V_vertex++;

	// makeUIVertex(V_vertex, tpx + (float)(TexturesMap[id].width) * Blk.sx, tpy + (float)(TexturesMap[id].height) * Blk.sy, Blk.x + TexturesMap[id].width, Blk.y + TexturesMap[id].height);
	makeUIVertex(V_vertex, tpx + (float)(TexturesMap[id].width) * Blk.sx, tpy + (float)(TexturesMap[id].height) * Blk.sy, 1, 1);
	V_vertex++;
	
	ModelsMap[id]->indices[0] = 0;
	ModelsMap[id]->indices[1] = 1;
	ModelsMap[id]->indices[2] = 2;
	ModelsMap[id]->indices[3] = 1;
	ModelsMap[id]->indices[4] = 2;
	ModelsMap[id]->indices[5] = 3;

	std::cout << "[UI DEBUG] init mesh" << std::endl;

	ModelsMap[id]->initMesh(BP, &VD, false);
}

inline void UIMaker::createUIDescriptorSets() {
	for (auto T : TexturesMap) {
		DescriptorSet temp;
		temp.init(BP, &DSL, {T.second.t.getViewAndSampler()});
		DSMap[T.first] = temp;
	}
}

inline void UIMaker::pipelinesAndDescriptorSetsInit() {
	std::cout << "[UI DEBUG] UI pipelines and descriptor sets init" << std::endl;
	RP.create();
	P.create(&RP);
	createUIDescriptorSets();
}

inline void UIMaker::pipelinesAndDescriptorSetsCleanup() {
	std::cout << "[UI DEBUG] UI pipelines and descript sets cleanup" << std::endl;
	P.cleanup();
	RP.cleanup();
	for(auto DS : DSMap)
		DS.second.cleanup();
}

inline void UIMaker::localCleanup() {
	for(auto T : TexturesMap)
		T.second.t.cleanup();
	
	for(auto M : ModelsMap)
		M.second->cleanup();

	DSL.cleanup();
	
	P.destroy();
	RP.destroy();
}

inline void UIMaker::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
	UIMaker *T = ((UIMakerAndModel *)Params)->ui;
	T->populateCommandBuffer(commandBuffer, currentImage);
}

inline void UIMaker::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
	RP.begin(commandBuffer, currentImage);
	P.bind(commandBuffer);
	for (auto M : ModelsMap)
		M.second->bind(commandBuffer);
	for (auto DS : DSMap)
		DS.second.bind(commandBuffer, P, 0, currentImage);
	
	for(auto& Blk : UIElementsMap) {				
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ModelsMap[Blk.first]->indices.size()), 1, 0, 0, 0);
	}

	RP.end(commandBuffer);			
}

inline void UIMaker::freeCommandBuffer(void *Params) {
	Model *M = ((UIMakerAndModel *)Params)->M;
	M->cleanup();
	
	free(Params);
}

inline void UIMaker::updateCommandBuffer() {
	std::cout << "[UI DEBUG] UI update command buffer" << std::endl;
	debugPrint();
	if (commandBufferMustUpdate) {
		for (auto DS : DSMap) {
			createUIMesh(DS.first);	// creates the new mesh
		
			UIMakerAndModel *uim = (UIMakerAndModel *)malloc(sizeof(UIMakerAndModel));
			uim->ui = this;
			uim->M = ModelsMap[DS.first];
			BP->submitCommandBuffer("ui" + std::to_string(DS.first), submitOrder,UIMaker::populateCommandBufferAccess, uim, UIMaker::freeCommandBuffer);

			commandBufferMustUpdate = false;
		}
	}
}

