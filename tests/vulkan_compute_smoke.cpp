#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"
#include "platform/vulkan/rendering/descriptors/VKDescriptorSetBinding.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

using namespace Prehistoric;

static uint32_t validationErrors = 0;

static void Check(VkResult result)
{
	if (result != VK_SUCCESS)
		throw std::runtime_error("Vulkan call failed: " + std::to_string(result));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user)
{
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		validationErrors++;
		fprintf(stderr, "%s\n", data->pMessage);
	}
	return VK_FALSE;
}

struct ComputeResources
{
	VkDevice device = VK_NULL_HANDLE;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkShaderModule module = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;

	~ComputeResources()
	{
		if (device == VK_NULL_HANDLE)
			return;

		vkDeviceWaitIdle(device);
		vkDestroyFence(device, fence, nullptr);
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyShaderModule(device, module, nullptr);
		vkDestroyPipelineLayout(device, layout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroyDevice(device, nullptr);
	}
};

static void RunCompute(VkPhysicalDevice gpu, uint32_t family, const std::vector<uint32_t>& code)
{
	ComputeResources resources;
	uint32_t count = 0;
	Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> extensions(count);
	Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, extensions.data()));
	std::vector<const char*> enabled;
	for (const auto& extension : extensions)
	{
		if (!strcmp(extension.extensionName, "VK_KHR_portability_subset"))
			enabled.push_back("VK_KHR_portability_subset");
	}

	float priority = 1.0f;
	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = family;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priority;
	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = (uint32_t)enabled.size();
	deviceInfo.ppEnabledExtensionNames = enabled.data();
	Check(vkCreateDevice(gpu, &deviceInfo, nullptr, &resources.device));
	VkDevice device = resources.device;
	VkQueue queue;
	vkGetDeviceQueue(device, family, 0, &queue);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = 256 * sizeof(uint32_t);
	bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	Check(vkCreateBuffer(device, &bufferInfo, nullptr, &resources.buffer));
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device, resources.buffer, &requirements);
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);
	uint32_t memoryType = UINT32_MAX;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if ((requirements.memoryTypeBits & (1u << i)) && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
		{
			memoryType = i;
			break;
		}
	}
	if (memoryType == UINT32_MAX)
		throw std::runtime_error("No coherent host-visible memory for compute readback!");

	VkMemoryAllocateInfo allocation = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocation.allocationSize = requirements.size;
	allocation.memoryTypeIndex = memoryType;
	Check(vkAllocateMemory(device, &allocation, nullptr, &resources.memory));
	Check(vkBindBufferMemory(device, resources.buffer, resources.memory, 0));

	VKDescriptorSetBinding binding(StorageBuffer, 0, COMPUTE_SHADER);
	binding.finalise();
	if (binding.getBinding().stageFlags != VK_SHADER_STAGE_COMPUTE_BIT)
		throw std::runtime_error("Compute descriptor stage flags are incorrect!");
	VkDescriptorSetLayoutCreateInfo setInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	setInfo.bindingCount = 1;
	setInfo.pBindings = &binding.getBinding();
	Check(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &resources.setLayout));
	VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 };
	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	Check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &resources.descriptorPool));
	VkDescriptorSetAllocateInfo setAllocation = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	setAllocation.descriptorPool = resources.descriptorPool;
	setAllocation.descriptorSetCount = 1;
	setAllocation.pSetLayouts = &resources.setLayout;
	VkDescriptorSet set;
	Check(vkAllocateDescriptorSets(device, &setAllocation, &set));
	VkDescriptorBufferInfo descriptorBuffer = { resources.buffer, 0, bufferInfo.size };
	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = set;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.pBufferInfo = &descriptorBuffer;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

	VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &resources.setLayout;
	Check(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &resources.layout));
	VkShaderModuleCreateInfo moduleInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleInfo.codeSize = code.size() * sizeof(uint32_t);
	moduleInfo.pCode = code.data();
	Check(vkCreateShaderModule(device, &moduleInfo, nullptr, &resources.module));
	VkPipelineShaderStageCreateInfo stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage.module = resources.module;
	stage.pName = "main";
	VKComputePipeline pipeline(device, stage, resources.layout);
	pipeline.setInvocationSize({ 4, 1, 1 });
	bool rejected = false;
	try { pipeline.RenderPipeline(); }
	catch (const std::runtime_error&) { rejected = true; }
	if (!rejected)
		throw std::runtime_error("Dispatch without a bound command buffer was accepted!");

	VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.queueFamilyIndex = family;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	Check(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &resources.commandPool));
	VkCommandBufferAllocateInfo commandAllocation = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandAllocation.commandPool = resources.commandPool;
	commandAllocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandAllocation.commandBufferCount = 1;
	VkCommandBuffer command;
	Check(vkAllocateCommandBuffers(device, &commandAllocation, &command));
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	Check(vkCreateFence(device, &fenceInfo, nullptr, &resources.fence));

	for (uint32_t pass = 0; pass < 2; pass++)
	{
		if (pass)
		{
			pipeline.RecreatePipeline();
			Check(vkResetCommandBuffer(command, 0));
			Check(vkResetFences(device, 1, &resources.fence));
		}
		void* mapped;
		Check(vkMapMemory(device, resources.memory, 0, bufferInfo.size, 0, &mapped));
		memset(mapped, 0, bufferInfo.size);
		vkUnmapMemory(device, resources.memory);

		VkCommandBufferBeginInfo begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		Check(vkBeginCommandBuffer(command, &begin));
		pipeline.BindPipeline(command);
		vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, resources.layout, 0, 1, &set, 0, nullptr);
		pipeline.RenderPipeline();
		pipeline.UnbindPipeline();
		VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		Check(vkEndCommandBuffer(command));
		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &command;
		Check(vkQueueSubmit(queue, 1, &submit, resources.fence));
		Check(vkWaitForFences(device, 1, &resources.fence, VK_TRUE, 30000000000ull));
		Check(vkMapMemory(device, resources.memory, 0, bufferInfo.size, 0, &mapped));
		bool correct = true;
		for (uint32_t i = 0; i < 256; i++)
			correct &= ((uint32_t*)mapped)[i] == i * 3 + 7;
		vkUnmapMemory(device, resources.memory);
		if (!correct)
			throw std::runtime_error("Compute readback did not match the expected values!");
	}
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(gpu, &properties);
	printf("%s: 256 values verified, pipeline recreation verified\n", properties.deviceName);
}

int main(int argc, char** argv)
{
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
	int result = 0;
	try
	{
		if (argc != 2)
			throw std::runtime_error("Usage: VulkanComputeSmoke shader.spv");
		std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
		if (!file || file.tellg() <= 0 || (size_t)file.tellg() % sizeof(uint32_t))
			throw std::runtime_error("Could not read the compute SPIR-V shader!");
		std::vector<uint32_t> code((size_t)file.tellg() / sizeof(uint32_t));
		file.seekg(0);
		if (!file.read((char*)code.data(), code.size() * sizeof(uint32_t)))
			throw std::runtime_error("Could not read the complete compute shader!");

		uint32_t count = 0;
		Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
		std::vector<VkExtensionProperties> extensions(count);
		Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
		std::vector<const char*> enabled;
		VkInstanceCreateInfo info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		for (const auto& extension : extensions)
		{
			if (!strcmp(extension.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
			{
				enabled.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
				info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
			}
			if (!strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				enabled.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		Check(vkEnumerateInstanceLayerProperties(&count, nullptr));
		std::vector<VkLayerProperties> layers(count);
		Check(vkEnumerateInstanceLayerProperties(&count, layers.data()));
		const char* validation = "VK_LAYER_KHRONOS_validation";
		for (const auto& layer : layers)
		{
			if (!strcmp(layer.layerName, validation))
			{
				info.enabledLayerCount = 1;
				info.ppEnabledLayerNames = &validation;
			}
		}
		if (!info.enabledLayerCount)
			throw std::runtime_error("Install the Vulkan SDK validation layer to run this check!");
		VkApplicationInfo application = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		application.apiVersion = VK_API_VERSION_1_1;
		info.pApplicationInfo = &application;
		info.enabledExtensionCount = (uint32_t)enabled.size();
		info.ppEnabledExtensionNames = enabled.data();
		Check(vkCreateInstance(&info, nullptr, &instance));
		auto createMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (!createMessenger)
			throw std::runtime_error("Vulkan debug utils are required for this check!");
		VkDebugUtilsMessengerCreateInfoEXT debug = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		debug.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug.pfnUserCallback = DebugMessage;
		Check(createMessenger(instance, &debug, nullptr, &messenger));

		Check(vkEnumeratePhysicalDevices(instance, &count, nullptr));
		std::vector<VkPhysicalDevice> devices(count);
		Check(vkEnumeratePhysicalDevices(instance, &count, devices.data()));
		uint32_t tested = 0;
		for (VkPhysicalDevice gpu : devices)
		{
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
			std::vector<VkQueueFamilyProperties> families(count);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, families.data());
			for (uint32_t i = 0; i < count; i++)
			{
				if (families[i].queueCount && (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					RunCompute(gpu, i, code);
					tested++;
					break;
				}
			}
		}
		if (!tested)
			throw std::runtime_error("No Vulkan compute device was found!");
	}
	catch (const std::exception& exception)
	{
		fprintf(stderr, "%s\n", exception.what());
		result = 1;
	}
	if (messenger)
		((PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))(instance, messenger, nullptr);
	if (instance)
		vkDestroyInstance(instance, nullptr);
	if (validationErrors)
		result = 1;
	printf("Vulkan validation errors: %u\n", validationErrors);
	return result;
}
