#pragma once
#include <vulkan.h>

class Vulkan
{
	const char* appName;
	const char* engineName;
	uint32_t imgWidth;
	uint32_t imgHeight;
	uint32_t WORKGROUP_SIZE = 32;
	uint32_t apiVersion;
	uint32_t queueFamilyIndex;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	uint32_t bufferSize;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkShaderModule computeShaderModule = VK_NULL_HANDLE;
	size_t computeShaderSize;
	uint32_t* computeShaderCode;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline computePipeLine = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	void printSupportedApiVersion();
	void printPhysicalDevices();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createInstance();
	void choosePhysicalDevice();
	void createLogicalDevice();
	void createBuffer();
	void createDescriptorSetLayout();
	void createDescriptorSet();
	void loadComputeShader(const char* fileName);
	void createComputePipeline();
	void createCommandBuffer();
	void submitCommandBuffer();
	int saveImage(const char* filename);
public:
	Vulkan(const char* _appName, const char* _engineName, uint32_t _imgWidth, uint32_t _imgHeight);
	~Vulkan();
};