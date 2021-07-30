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


namespace Marvel {

	class mvGraphicsContext;

	struct UniformBufferObject 
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class mvDevice
	{

		struct QueueFamilyIndices {

			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

	public:

		mvDevice() = default;
		mvDevice(GLFWwindow* window);
		~mvDevice();

		void finish();

		void present(mvGraphicsContext&);

		VkDevice      getDevice();
		VkExtent2D    getSwapChainExtent();
		VkDescriptorSetLayout* getDescriptorSetLayout();
		VkRenderPass getRenderPass();
		VkPipelineLayout* getPipelineLayout();
		VkPipeline*       getPipeline();

		std::uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(mvGraphicsContext&, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		// new uses
		VkShaderModule createShaderModule(const std::vector<char>& code);
		void createCommandPool();
		void createDepthResources();
		void allocCommandBuffers();
		void createCommandBuffers(mvGraphicsContext& graphics, int buffer);
		void createTextureImage();
		void createTextureImageView();
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void createImage(uint32_t width, uint32_t height, VkFormat format, 
			VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkImage& image, VkDeviceMemory& imageMemory);

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void createTextureSampler();

	private:

		void createVulkanInstance();
		void setupDebugMessenger();
		void createSurface(GLFWwindow* window);
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain(GLFWwindow* window);
		void createImageViews();
		void createRenderPass();
		void createDescriptorSetLayout();
		void createFrameBuffers();
		void createUniformBuffers();
		void updateUniformBuffer(uint32_t currentImage);
		void createDescriptorPool();
		void createDescriptorSets();
		void createSyncObjects();

		// helpers
		bool                    checkValidationLayerSupport();
		bool                    isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device);
		bool                    checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		

	private:


		std::vector<VkBuffer>       _uniformBuffers;
		std::vector<VkDeviceMemory> _uniformBuffersMemory;
		VkImage                     _textureImage;
		VkDeviceMemory              _textureImageMemory;
		VkImageView                 _textureImageView;
		VkSampler                   _textureSampler;

		// depth buffer
		VkImage        _depthImage;
		VkDeviceMemory _depthImageMemory;
		VkImageView    _depthImageView;

		// options
		bool _enableValidationLayers = true;

		VkInstance                   _instance;
		VkDebugUtilsMessengerEXT     _debugMessenger;
		VkSurfaceKHR                 _surface;
		VkPhysicalDevice             _physicalDevice = VK_NULL_HANDLE;
		VkDevice                     _device;
		VkQueue                      _graphicsQueue;
		VkQueue                      _presentQueue;
		VkSwapchainKHR               _swapChain;
		std::vector<VkImage>         _swapChainImages;
		std::vector<VkImageView>     _swapChainImageViews;
		VkFormat                     _swapChainImageFormat;
		VkExtent2D                   _swapChainExtent;
		VkRenderPass                 _renderPass;
		std::vector<VkFramebuffer>   _swapChainFramebuffers;
		std::vector<VkSemaphore>     _imageAvailableSemaphores;
		std::vector<VkSemaphore>     _renderFinishedSemaphores;
		std::vector<VkFence>         _inFlightFences;
		std::vector<VkFence>         _imagesInFlight;
		VkDescriptorSetLayout        _descriptorSetLayout;
		VkDescriptorPool             _descriptorPool;
		std::vector<VkDescriptorSet> _descriptorSets;
		VkCommandPool                _commandPool;
		std::vector<VkCommandBuffer> _commandBuffers;
		VkPipelineLayout             _pipelineLayout;
		VkPipeline                   _pipeline;

		size_t                       _currentFrame = 0;

		const std::vector<const char*> _validationLayers = { "VK_LAYER_KHRONOS_validation"};
		const std::vector<const char*> _deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		const int                      _max_frames_in_flight = 2;
	};

}