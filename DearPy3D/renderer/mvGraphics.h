#pragma once

#include <vector>
#include <array>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mvVertexBuffer.h"
#include "mvIndexBuffer.h"
#include "mvPipeline.h"
#include "mvAllocator.h"
#include "mvDeletionQueue.h"
#include "mvImGuiManager.h"
#include "mvPhysicalDevice.h"
#include "mvLogicalDevice.h"
#include "mvSwapChain.h"

namespace DearPy3D {

	class mvGraphics
	{

	public:

		mvGraphics(GLFWwindow* window);

		void cleanup();
		void recreateSwapChain(float width, float height);

		mvSwapChain&      getSwapChain()           { return _swapChain; }
		mvLogicalDevice&  getLogicalDevice()       { return _logicalDevice; }
		mvPhysicalDevice& getPhysicalDevice()      { return _physicalDevice; }
		VkInstance        getInstance()            { return _instance; }
		VkDescriptorPool  getDescriptorPool()      { return _descriptorPool; }
		mvDeletionQueue&  getDeletionQueue()       { return _deletionQueue; }

		// descriptor pool
		void allocateDescriptorSet(mvDescriptorSet* descriptorSet, mvDescriptorSetLayout& layout);

		void setCamera(glm::mat4 camera) { _camera = camera; }
		void setProjection(glm::mat4 projection) { _projection = projection; }
		glm::mat4 getCamera() const { return _camera; }
		glm::mat4 getProjection() const { return _projection; }
		
	private:

		// initialization
		void createVulkanInstance();
		void setupDebugMessenger();
		void createDescriptorPool();

		// internal helpers
		bool checkValidationLayerSupport();
		
		
	private:

		glm::mat4 _camera;
		glm::mat4 _projection;

		mvLogicalDevice                _logicalDevice;
		mvPhysicalDevice               _physicalDevice;
		mvSwapChain                    _swapChain;
		mvDeletionQueue                _deletionQueue;
		bool                           _enableValidationLayers = true;
		VkInstance                     _instance;
		VkDebugUtilsMessengerEXT       _debugMessenger;
		VkDescriptorPool               _descriptorPool;
		const std::vector<const char*> _validationLayers = { "VK_LAYER_KHRONOS_validation" };
	};

}