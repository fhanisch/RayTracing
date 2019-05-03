#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Vulkan.h"

Vulkan::Vulkan(const char* _appName, const char* _engineName, uint32_t _imgWidth, uint32_t _imgHeight)
{
	appName = _appName;
	engineName = _engineName;
	imgWidth = _imgWidth;
	imgHeight = _imgHeight;
	apiVersion = 0;
	bufferSize = 4 * sizeof(float) * imgWidth * imgHeight;

	std::cout << "\nInit Vulkan..." << std::endl;
	printSupportedApiVersion();
	createInstance();
	printPhysicalDevices();
	choosePhysicalDevice();
	createLogicalDevice();
	createBuffer();
	createDescriptorSetLayout();
	createDescriptorSet();
	loadComputeShader("C:/Home/Entwicklung/RayTracing/x64/Debug/spheres.spv");
	createComputePipeline();
	createCommandBuffer();
	submitCommandBuffer();
	saveImage("Kugeln.ppm");
}

Vulkan::~Vulkan()
{
	std::cout << "Close Vulkan." << std::endl;
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyPipeline(device, computePipeLine, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device, computeShaderModule, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkFreeMemory(device, bufferMemory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void Vulkan::printSupportedApiVersion()
{
	vkEnumerateInstanceVersion(&apiVersion);
	std::cout << "Supported Api Version: " << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "." << VK_VERSION_PATCH(apiVersion) << std::endl;
}

void Vulkan::printPhysicalDevices()
{
	if (instance == VK_NULL_HANDLE) {
		std::cout << "Create Instance first!" << std::endl;
		return;
	}
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		std::cout << "Failed to find GPUs with Vulkan support!" << std::endl;
		return;
	}
	VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
	std::cout << "\nCount of Devices: " << deviceCount << std::endl << std::endl;
	for (uint32_t i = 0; i < deviceCount; i++) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, nullptr);
		VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
		vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, extensions);
		std::cout << "Device #" << i << " Extensions:" << std::endl;
		for (uint32_t j = 0; j < extensionCount; j++) {
			std::cout << "\t#" << j << "\t" << extensions[j].extensionName << std::endl;
		}
		delete[] extensions;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		std::cout << "Device #" << i << " Properties:" << std::endl;
		std::cout << "\t" << "Api Version:    " << deviceProperties.apiVersion << std::endl;
		std::cout << "\t" << "Device ID:      " << deviceProperties.deviceID << std::endl;
		std::cout << "\t" << "Device Name:    " << deviceProperties.deviceName << std::endl;
		std::cout << "\t" << "Device Type:    " << deviceProperties.deviceType << std::endl;
		std::cout << "\t" << "Driver Version: " << deviceProperties.driverVersion << std::endl;
		std::cout << "\t" << "Vendor ID:      " << deviceProperties.vendorID << std::endl;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, nullptr);
		VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);
		std::cout << "Device #" << i << " Queue Families:" << std::endl;
		for (uint32_t j = 0; j < queueFamilyCount; j++)
		{
			std::cout << "\t" << "Queue Family #" << j << ":" << std::endl;
			std::cout << "\t\t" << "VK_QUEUE_GRAPHICS_BIT:           " << ((queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
			std::cout << "\t\t" << "VK_QUEUE_COMPUTE_BIT:            " << ((queueFamilies[j].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
			std::cout << "\t\t" << "VK_QUEUE_TRANSFER_BIT:           " << ((queueFamilies[j].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
			std::cout << "\t\t" << "VK_QUEUE_SPARSE_BINDING_BIT:     " << ((queueFamilies[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
			std::cout << "\t\t" << "Queue Count:                     " << queueFamilies[j].queueCount << std::endl;
			std::cout << "\t\t" << "Timestamp Valid Bits:            " << queueFamilies[j].timestampValidBits << std::endl;
			uint32_t width = queueFamilies[j].minImageTransferGranularity.width;
			uint32_t height = queueFamilies[j].minImageTransferGranularity.height;
			uint32_t depth = queueFamilies[j].minImageTransferGranularity.depth;
			std::cout << "\t\t" << "Min Image Timestamp Granularity: " << width << ", " << height << ", " << depth << std::endl;
		}
		delete[] queueFamilies;
	}
	delete[] devices;
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	std::cout << "Failed to find suitable memory type!" << std::endl;
	exit(1);
}

void Vulkan::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = engineName;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	const unsigned int validationLayerCount = 1;
	const char* validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
	const unsigned int globalExtensionCount = 0;
	const char** globalExtensions = nullptr;
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = validationLayerCount;
	createInfo.ppEnabledLayerNames = validationLayers;
	createInfo.enabledExtensionCount = globalExtensionCount;
	createInfo.ppEnabledExtensionNames = globalExtensions;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		std::cout << "Failed to create instance!" << std::endl;
		exit(1);
	}
}

void Vulkan::choosePhysicalDevice()
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		std::cout << "Failed to find GPUs with Vulkan support!" << std::endl;
		exit(1);
	}
	VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
	
	physicalDevice = devices[1];

	if (physicalDevice == VK_NULL_HANDLE) {
		std::cout << "Failed to find a suitable GPU!" << std::endl;
		exit(1);
	}
	delete[] devices;
}

void Vulkan::createLogicalDevice()
{
	queueFamilyIndex = 2;
	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.flags = 0;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	const unsigned int deviceExtensionCount = 0;
	const char** deviceExtensions = nullptr;
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL; // evtl. auch hier den validationLayer eintragen
	createInfo.enabledExtensionCount = deviceExtensionCount;
	createInfo.ppEnabledExtensionNames = deviceExtensions;
	createInfo.pEnabledFeatures = &deviceFeatures;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		std::cout << "Failed to create logical device!" << std::endl;
		exit(1);
	}

	vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

void Vulkan::createBuffer()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		std::cout << "Failed to create vertex buffer!" << std::endl;
		exit(1);
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		std::cout << "Failed to allocate vertex buffer memory!" << std::endl;
		exit(1);
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Vulkan::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = 1; 
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
 
	vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
}

void Vulkan::createDescriptorSet()
{
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

	vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

	vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);

	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = bufferSize;

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

void Vulkan::loadComputeShader(const char* fileName)
{
	FILE* file = fopen(fileName, "rb");
	if (file == NULL) {
		std::cout << "Could not open " << fileName << "!" << std::endl;
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	computeShaderSize = ftell(file);
	rewind(file);
	computeShaderCode = (uint32_t*)malloc(computeShaderSize);
	fread((void*)computeShaderCode, computeShaderSize, 1, file);
	fclose(file);
}

void Vulkan::createComputePipeline()
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = computeShaderCode;
	createInfo.codeSize = computeShaderSize;

	vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule);
	free(computeShaderCode);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageCreateInfo.module = computeShaderModule;
	shaderStageCreateInfo.pName = "main";

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);

	VkComputePipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stage = shaderStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;

	if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &computePipeLine) != VK_SUCCESS)
	{
		std::cout << "Create Compute Pipeline failed !" << std::endl;
		exit(1);
	}
	std::cout << "Compute Pipeline created." << std::endl;
}

void Vulkan::createCommandBuffer()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
	vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool; 
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	{

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeLine);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		vkCmdDispatch(commandBuffer, (uint32_t)ceil(imgWidth / float(WORKGROUP_SIZE)), (uint32_t)ceil(imgHeight / float(WORKGROUP_SIZE)), 1);
	}
	vkEndCommandBuffer(commandBuffer);
}

void Vulkan::submitCommandBuffer()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	vkCreateFence(device, &fenceCreateInfo, NULL, &fence);

	vkQueueSubmit(queue, 1, &submitInfo, fence);

	vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(device, fence, NULL);
}

int Vulkan::saveImage(const char* filename)
{
	FILE* file;
	const char* magic = "P6\n";
	const char* width = "3840\n";
	const char* height = "2160\n";
	const char* hellikeit = "255\n";
	float* data;
	size_t pixelSize = 3 * imgWidth * imgHeight;
	char* pixels = (char*)calloc(1, pixelSize);
	char* pixelData = pixels;
	memset(pixels, 255, pixelSize);
	
	vkMapMemory(device, bufferMemory, 0, bufferSize, 0, (void**)&data);
	{
		for (unsigned int i = 0; i < imgWidth * imgHeight; i++)
		{
			pixels[0] = (char)(data[0]*255);
			pixels[1] = (char)(data[1]*255);
			pixels[2] = (char)(data[2]*255);
			pixels += 3;
			data += 4;
		}
	}
	vkUnmapMemory(device, bufferMemory);
	
	file = fopen(filename, "wb");
	fwrite(magic, strlen(magic), 1, file);
	fwrite(width, strlen(width), 1, file);
	fwrite(height, strlen(height), 1, file);
	fwrite(hellikeit, strlen(hellikeit), 1, file);
	fwrite(pixelData, pixelSize, 1, file);
	fclose(file);

	return 0;
}