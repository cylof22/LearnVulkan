/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

#include "common\vulkan_wrapper.h"
#include "VulkanApplication.h"
#include "VulkanRenderer.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanHardwareIndexBuffer.h"
#include "VulkanHardwareUniformBuffer.h"
#include "VulkanHardwareTextureBuffer.h"
#include "VulkanGpuProgram.h"
#include "VulkanDevice.h"
#include "VulkanRenderable.h"
#include "VulkanPipelineState.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanDescriptorSetMgr.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

//#include <vector>
#include <thread>


/**
* Our saved state data.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

/**
* Shared state for our app.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;
	VulkanApplication* m_pApp;
	int animating;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

#include "Stb\stb_image.h"

void addSkyboxEntity(const VulkanDevice* pDevice, VulkanRenderer* pRenderer, ANativeWindow* pWnd, AAssetManager* pAssetMgr)
{
	float skyboxGeometry[] = {
		-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};

	std::vector<std::pair<VkFormat, VkDeviceSize>> skyboxDescriptions;
	skyboxDescriptions.emplace_back(std::make_pair(VK_FORMAT_R32G32B32A32_SFLOAT, 0));
	skyboxDescriptions.emplace_back(std::make_pair(VK_FORMAT_R32G32B32_SFLOAT, 16));

	std::shared_ptr<VulkanHardwareVertexBuffer> skyboxVertexBuffer = std::make_shared<VulkanHardwareVertexBuffer>(pDevice->getGPU(), pDevice->getGraphicDevice(),
		(void*)skyboxGeometry, sizeof(skyboxGeometry), sizeof(float) * 7, skyboxDescriptions);

	const std::string skyboxVertexProg =
		"#version 450\n"
		"layout(push_constant) uniform mvpBuffer {\n"
		"	mat4 mvp;\n"
		"} transform;\n"
		"layout(location = 0) in vec4 pos;\n"
		"layout(location = 1) in vec3 i_UV;\n"
		"layout(location = 0) out vec3 o_UV;\n"
		"void main() {\n"
		"   o_UV = mat3(transform.mvp) * i_UV;"
		"   gl_Position = pos;\n"
		"}\n";

	std::shared_ptr<VulkanGpuProgram> skyboxVertexShaderProg = std::make_shared<VulkanGpuProgram>(pDevice->getGraphicDevice(),
		"skyboxVertexshader.vert", shaderc_glsl_vertex_shader, skyboxVertexProg);

	const std::string skyboxFragmentProg = 
		"#version 450\n"
		"layout(binding = 1) uniform samplerCube tex;"
		"layout(location = 0) in vec3 o_UV;\n"
		"layout(location = 0) out vec4 fragColor;\n"
		"void main() {\n"
		"   fragColor = texture(tex, o_UV);\n"
		"}\n";

	std::shared_ptr<VulkanGpuProgram> skyboxFragShaderProg = std::make_shared<VulkanGpuProgram>(pDevice->getGraphicDevice(),
		"fragshader.vert", shaderc_glsl_fragment_shader, skyboxFragmentProg);

	VulkanRenderable* skyboxEntity = new VulkanRenderable;
	skyboxEntity->setVertexBuffer(skyboxVertexBuffer);
	skyboxEntity->setVertexShader(skyboxVertexShaderProg);
	skyboxEntity->setFragmentShader(skyboxFragShaderProg);

	std::vector<std::string> skyboxFiles;
	skyboxFiles.emplace_back("Skybox_right1.png");
	skyboxFiles.emplace_back("Skybox_left2.png");
	skyboxFiles.emplace_back("Skybox_top3.png");
	skyboxFiles.emplace_back("Skybox_bottom4.png");
	skyboxFiles.emplace_back("Skybox_front5.png");
	skyboxFiles.emplace_back("Skybox_back6.png");

	std::vector<std::pair<char*, off_t>> skyboxImageData;
	off_t size;
	for (size_t i = 0; i < skyboxFiles.size(); i++)
	{
		AAsset* pTextureAsset = AAssetManager_open(pAssetMgr, skyboxFiles[i].c_str(), AASSET_MODE_UNKNOWN);

		off_t imageSize = AAsset_getLength(pTextureAsset);

		char* buffer = new char[imageSize];

		off_t readSize = AAsset_read(pTextureAsset, (void*)buffer, imageSize);
		assert(imageSize == readSize);
		size = readSize;

		skyboxImageData.emplace_back(buffer, size);
	}

	std::shared_ptr<VulkanHardwareTextureBuffer> rSkyboxTextureBuffer =
		std::make_shared<VulkanHardwareTextureBuffer>(pDevice, pRenderer->getCmdPool(), skyboxImageData, size);
		//std::make_shared<VulkanHardwareTextureBuffer>(pDevice, pRenderer->getCmdPool(), skyboxImageData[5].first, skyboxImageData[5].second, VK_IMAGE_USAGE_SAMPLED_BIT, true);

	skyboxEntity->setTextureBuffer(rSkyboxTextureBuffer);
	skyboxEntity->setTopologyType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	
	std::vector<VkDescriptorSetLayout> shaderParams;
	VulkanDescriptorSetMgr::get()->createDescriptorSetLayout(pDevice->getGraphicDevice(), skyboxEntity, shaderParams);

	VkDescriptorSet renderDescriptorSet;
	VulkanDescriptorSetMgr::get()->createDescriptorSet(pDevice->getGraphicDevice(), pRenderer->getDescriptorPool(), shaderParams, renderDescriptorSet);

	VulkanDescriptorSetMgr::get()->updateDescriptorSetbyRenderableEntity(pDevice->getGraphicDevice(), renderDescriptorSet, skyboxEntity);

	skyboxEntity->setDescriptorSet(renderDescriptorSet);
	skyboxEntity->setDescriptorSetLayout(shaderParams);
	skyboxEntity->addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4));

	pRenderer->getGraphicPipeline()->setViewport((float)ANativeWindow_getWidth(pWnd), (float)ANativeWindow_getHeight(pWnd));

	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendState.blendEnable = VK_FALSE;
	pRenderer->getGraphicPipeline()->addColorBlendAttachment(blendState);

	pRenderer->getGraphicPipeline()->setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VulkanGraphicPipelineState pipelineState;
	pipelineState.activeRenderPass = pRenderer->getRenderPass();
	VkPipeline renderPipeline = VK_NULL_HANDLE;
	VkPipelineLayout renderPipelineLayout = VK_NULL_HANDLE;
	pRenderer->getGraphicPipeline()->createGraphicPipeline(pWnd, skyboxEntity, pipelineState, renderPipeline, renderPipelineLayout);

	pRenderer->addRenderable(skyboxEntity, renderPipeline, renderPipelineLayout);
}


void addRenderableEntity(const VulkanDevice* pDevice, VulkanRenderer* pRenderer, ANativeWindow* pWnd, AAssetManager* pAssetMgr)
{
	// Maybe a bug of the driver, put the vertex data behind the program will cause crash
	/*const float vertexData[] = { 0.0f, -0.5f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -0.5f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 0.0f };*/

	//float vertexData[14 * 6];

	//for (size_t i = 0; i < 14; i++)
	//{
	//	int b = 1 << i;
	//	float x = (2.0 * float((0x287a & b) != 0) - 1.0) * 1.0;
	//	float y = (2.0 * float((0x02af & b) != 0) - 1.0) * 1.0;
	//	float z = (2.0 * float((0x31e3 & b) != 0) - 1.0) * 1.0;

	//	vertexData[i * 6 + 0] = x;
	//	vertexData[i * 6 + 1] = y;
	//	vertexData[i * 6 + 2] = z;
	//	vertexData[i * 6 + 3] = 1.0;
	//	vertexData[i * 6 + 4] = x;
	//	vertexData[i * 6 + 5] = y;
	//}

	const float vertexData[] = {
		-1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // -X side 
		-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

		-1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 2.0f, // -Z side 
		1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f,  2.0f,
		1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 2.0f,
		-1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 2.0f,
		-1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 2.0f,
		1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 2.0f,

		-1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 3.0f,// -Y 
		1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 3.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 3.0f,
		-1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 3.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 3.0f,
		-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f,

		-1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 4.0f, // +Y side 
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 4.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 4.0f,
		-1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 4.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 4.0f,
		1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 4.0f,

		1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 5.0f, // +X side 
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 5.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 5.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 5.0f,
		1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 5.0f,
		1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 5.0f,

		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // +Z side 
		-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
	};

	std::vector<std::pair<VkFormat, VkDeviceSize>> vertexDescriptions;
	vertexDescriptions.emplace_back(std::make_pair(VK_FORMAT_R32G32B32A32_SFLOAT, 0));
	vertexDescriptions.emplace_back(std::make_pair(VK_FORMAT_R32G32B32_SFLOAT, 16));

	//const float vertexData[] = { 
	//	1.f,-1.f,-1.f, 1.f, // 0
	//	0.f, 0.f,
	//	1.f, 1.f,-1.f, 1.f, // 1
	//	1.f, 0.f,
	//	-1.f,-1.f,-1.f, 1.f, // 2
	//	1.f, 1.f,
	//	-1.f, 1.f,-1.f, 1.f, // 3
	//	0.f, 1.f,

	//	1.f,-1.f, 1.f, 1.f, // 4
	//	0.f, 0.f,
	//	1.f, 1.f, 1.f, 1.f, // 5
	//	1.f, 0.f,
	//	-1.f,-1.f, 1.f, 1.f, // 6
	//	1.f, 1.f,
	//	-1.f, 1.f, 1.f, 1.f, // 7
	//	1.f, 0.f,
	//};

	// Only used for testing
	std::shared_ptr<VulkanHardwareVertexBuffer> vertexBuffer = std::make_shared<VulkanHardwareVertexBuffer>(pDevice->getGPU(), pDevice->getGraphicDevice(),
		(void*)vertexData, sizeof(vertexData), sizeof(float) * 7, vertexDescriptions);

	/*const uint16_t indexData[] = {
		0, 2, 3, 3, 1, 0,
		4, 5, 7, 7, 6, 4,
		0, 1, 5, 5, 4, 0,
		2, 6, 7, 7, 3, 2,
		0, 4, 6, 6, 2, 0,
		5, 1, 3, 3, 7, 5,
	};

	std::shared_ptr<VulkanHardwareIndexBuffer> indexBuffer = std::make_shared<VulkanHardwareIndexBuffer>(pDevice->getGPU(), pDevice->getGraphicDevice(),
		(void*)indexData, sizeof(indexData));*/

	const std::string vertShaderText =
		"#version 450\n"
		"layout(push_constant) uniform mvpBuffer {\n"
		"	mat4 mvp;\n"
		"} transform;\n"
		"layout(location = 0) in vec4 pos;\n"
		"layout(location = 1) in vec3 i_UV;\n"
		"layout(location = 0) out vec3 o_UV;\n"
		"void main() {\n"
		"   o_UV = i_UV;\n"
		"   gl_Position = transform.mvp * pos;\n"
		"}\n";

	std::shared_ptr<VulkanGpuProgram> vertexShaderProg = std::make_shared<VulkanGpuProgram>(pDevice->getGraphicDevice(),
		"vertexshader.vert", shaderc_glsl_vertex_shader, vertShaderText);

	const std::string fragShaderText =
		"#version 450\n"
		"layout(binding = 1) uniform samplerCube tex;"
		"layout(location = 0) in vec3 o_UV;\n"
		"layout(location = 0) out vec4 fragColor;\n"
		"void main() {\n"
		"   fragColor = texture(tex, o_UV);\n"
		"}\n";

	std::shared_ptr<VulkanGpuProgram> fragShaderProg = std::make_shared<VulkanGpuProgram>(pDevice->getGraphicDevice(),
		"fragshader.vert", shaderc_glsl_fragment_shader, fragShaderText);

	VulkanRenderable* renderEntity = new VulkanRenderable;
	renderEntity->setVertexBuffer(vertexBuffer);
	//renderEntity->setIndexBuffer(indexBuffer);
	renderEntity->setVertexShader(vertexShaderProg);
	renderEntity->setFragmentShader(fragShaderProg);
	renderEntity->setTopologyType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	glm::mat4 model, view, projection, clip;
	model = glm::mat4(1.0f);
	projection = glm::ortho(-5.f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
	view = glm::mat4(1.0f); /* glm::lookAt(
							glm::vec3(0, 3, -10),		// Camera is in World Space
							glm::vec3(0, 0, 0),		// and looks at the origin
							glm::vec3(0, -1, 0)		// Head is up
							);*/

	model = glm::translate(model, glm::vec3(-1.5f, 1.5f, -1.5f));

	// Vulkan clip space has inverted Y and half Z. 
	// clang-format off 
	clip = glm::mat4(1.0f); /* glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
							0.0f,-1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.5f, 0.0f,
							0.0f, 0.0f, 0.5f, 1.0f);*/


	glm::mat4 imvp = clip * projection * view * model;

	std::shared_ptr<VulkanHardwareUniformBuffer> mvpUniformBuffer = std::make_shared<VulkanHardwareUniformBuffer>(pDevice->getGPU(),
		pDevice->getGraphicDevice(), &imvp, sizeof(imvp));

	renderEntity->setUniformBuffer(mvpUniformBuffer);

	std::shared_ptr<VulkanHardwareTextureBuffer> rTextureBuffer = nullptr;
	std::vector<std::string> skyboxFiles;
	skyboxFiles.emplace_back("Skybox_right1.png");
	skyboxFiles.emplace_back("Skybox_left2.png");
	skyboxFiles.emplace_back("Skybox_top3.png");
	skyboxFiles.emplace_back("Skybox_bottom4.png");
	skyboxFiles.emplace_back("Skybox_front5.png");
	skyboxFiles.emplace_back("Skybox_back6.png");

	std::vector<std::pair<char*, long>> skyboxImageData;
	for (size_t i = 0; i < skyboxFiles.size(); i++)
	{
		off_t size;
		AAsset* pTextureAsset = AAssetManager_open(pAssetMgr, skyboxFiles[i].c_str(), AASSET_MODE_UNKNOWN);

		off_t imageSize = AAsset_getLength(pTextureAsset);

		char* buffer = new char[imageSize];

		off_t readSize = AAsset_read(pTextureAsset, (void*)buffer, imageSize);
		assert(imageSize == readSize);
		size = readSize;

		skyboxImageData.emplace_back(buffer, size);
	}

	rTextureBuffer = std::make_shared<VulkanHardwareTextureBuffer>(pDevice, pRenderer->getCmdPool(), skyboxImageData);
	if (rTextureBuffer)
		renderEntity->setTextureBuffer(rTextureBuffer);

	std::vector<VkDescriptorSetLayout> shaderParams;
	VulkanDescriptorSetMgr::get()->createDescriptorSetLayout(pDevice->getGraphicDevice(), renderEntity, shaderParams);

	VkDescriptorSet renderDescriptorSet;
	VulkanDescriptorSetMgr::get()->createDescriptorSet(pDevice->getGraphicDevice(), pRenderer->getDescriptorPool(), shaderParams, renderDescriptorSet);

	VulkanDescriptorSetMgr::get()->updateDescriptorSetbyRenderableEntity(pDevice->getGraphicDevice(), renderDescriptorSet, renderEntity);

	renderEntity->setDescriptorSet(renderDescriptorSet);
	renderEntity->setDescriptorSetLayout(shaderParams);

	VulkanGraphicPipelineState pipelineState;
	pipelineState.activeRenderPass = pRenderer->getRenderPass();
	VkPipeline renderPipeline = VK_NULL_HANDLE;
	VkPipelineLayout renderPipelineLayout = VK_NULL_HANDLE;
	pRenderer->getGraphicPipeline()->createGraphicPipeline(pWnd, renderEntity, pipelineState, renderPipeline, renderPipelineLayout);

	pRenderer->addRenderable(renderEntity, renderPipeline, renderPipelineLayout);
}
/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
	// initialize OpenGL ES and EGL
	engine->m_pApp = new VulkanApplication;
	std::vector<const char*> extensionNames = {
		VK_KHR_SURFACE_EXTENSION_NAME
		, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
		, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		/*VK_EXT_DEBUG_MARKER_EXTENSION_NAME*/
	};

	std::vector<const char*> layerNames = {
		"VK_LAYER_GOOGLE_threading"
		,"VK_LAYER_GOOGLE_unique_objects"
		,"VK_LAYER_LUNARG_parameter_validation"
		,"VK_LAYER_LUNARG_object_tracker"
		,"VK_LAYER_LUNARG_image"
		,"VK_LAYER_LUNARG_core_validation"
		,"VK_LAYER_LUNARG_swapchain"
		//,"VK_LAYER_LUNARG_api_dump"
		//,"VK_LAYER_LUNARG_device_limits"
	};

	engine->m_pApp->createVulkanInstance(layerNames, extensionNames, engine->app->window);

	addSkyboxEntity(engine->m_pApp->getDevice(), engine->m_pApp->getRender(), engine->app->window, engine->app->activity->assetManager);

	return 0;
}

/**
* Just the current frame in the display.
*/
static void engine_draw_frame(struct engine* engine) {

	// Just fill the screen with a color.
	if (engine->m_pApp && engine->m_pApp->getRender())
	{
		engine->m_pApp->getRender()->update();
		engine->m_pApp->getRender()->render();
	}
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine) {
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Also stop animating.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;
	InitVulkan();
	
	// loop waiting for stuff to do.
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						LOGI("accelerometer: x=%f y=%f z=%f",
							event.acceleration.x, event.acceleration.y,
							event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// Done with events; draw next animation frame.
			engine.state.angle += .01f;
			if (engine.state.angle > 1) {
				engine.state.angle = 0;
			}

			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			engine_draw_frame(&engine);
		}
	}
}
