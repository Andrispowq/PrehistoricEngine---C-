#include "Includes.hpp"
#include <glad/glad.h>
#include "VkShader.h"

namespace Prehistoric
{
	namespace ResourceLoader
	{
		std::vector<char> ResourceLoader::LoadShaderVK(const std::string& path)
		{
			const std::string SHADER_LOC = "res/shaders/";

			std::ifstream file;
			file.open((SHADER_LOC + path).c_str(), std::ios::ate | std::ios::binary);

			if (file.is_open())
			{
				size_t fileSize = (size_t)file.tellg();
				std::vector<char> buffer(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);

				file.close();
				return buffer;
			}
			else
			{
				PR_LOG_ERROR("Unable to load shader: %s\n", path.c_str());
			}

			return { -1 };
		}
	};

	VKShader::VKShader(Context* context, Swapchain* swapchain, const std::vector<char>* files, uint32_t length)
	{
		modules = new VkShaderModule[5];
		shaderStages = new VkPipelineShaderStageCreateInfo[5];

		this->physicalDevice = (VKPhysicalDevice*)context->getPhysicalDevice();
		this->device = (VKDevice*)context->getDevice();
		counter = 0;

		if (length == 1) //Compute shader
		{
			if (!AddShader(files[0], COMPUTE_SHADER))
				PR_LOG_RUNTIME_ERROR("Compute shader couldn't be added\n");
		}
		else if (length == 2) // Vertex + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}
		else if (length == 3) //Vertex + geometry + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], GEOMETRY_SHADER))
				PR_LOG_RUNTIME_ERROR("Geometry shader couldn't be added!");
			if (!AddShader(files[2], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}
		else if (length == 5) //Vertex + tessellation control + tessellation evaluation + geomtry + fragment shader
		{
			if (!AddShader(files[0], VERTEX_SHADER))
				PR_LOG_RUNTIME_ERROR("Vertex shader couldn't be added!");
			if (!AddShader(files[1], TESSELLATION_CONTROL_SHADER))
				PR_LOG_RUNTIME_ERROR("Tessellation control shader couldn't be added!");
			if (!AddShader(files[2], TESSELLATION_EVALUATION_SHADER))
				PR_LOG_RUNTIME_ERROR("Tessellation evaluation shader couldn't be added!");
			if (!AddShader(files[3], GEOMETRY_SHADER))
				PR_LOG_RUNTIME_ERROR("Geometry shader couldn't be added!");
			if (!AddShader(files[4], FRAGMENT_SHADER))
				PR_LOG_RUNTIME_ERROR("Fragment shader couldn't be added!");
		}

		this->swapchain = (VKSwapchain*)swapchain;
		this->descriptorPool = new VKDescriptorPool(physicalDevice, device, this->swapchain);
	}

	VKShader::VKShader(Context* context, Swapchain* swapchain)
	{
		modules = new VkShaderModule[5];
		shaderStages = new VkPipelineShaderStageCreateInfo[5];

		this->physicalDevice = (VKPhysicalDevice*)context->getPhysicalDevice();
		this->device = (VKDevice*)context->getDevice();
		counter = 0;

		this->swapchain = (VKSwapchain*)swapchain;
		this->descriptorPool = new VKDescriptorPool(physicalDevice, device, this->swapchain);
	}

	VKShader::~VKShader()
	{
		vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);

		delete descriptorPool;

		for (size_t i = 0; i < counter; i++)
		{
			vkDestroyShaderModule(device->getDevice(), modules[i], nullptr);
		}

		delete[] modules;
		delete[] shaderStages;
	}

	/*
		We only add every uniform once, so we could use the index 0 everywhere, but upon creation the instance_counter is 0, so we'll use that
	*/
	bool VKShader::AddUniform(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, size_t size, Texture* texture)
	{
		descriptorPool->addUniform(name, stages, type, set, binding, (uint32_t)size, texture);

		return true;
	}

	bool VKShader::AddShader(const std::vector<char>& code, ShaderType type)
	{
		VkShaderModule module = CreateShaderModule(code);
		VkPipelineShaderStageCreateInfo info = {};

		VkShaderStageFlagBits flag;

		switch (type)
		{
		case VERTEX_SHADER:
			flag = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case TESSELLATION_CONTROL_SHADER:
			flag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			break;
		case TESSELLATION_EVALUATION_SHADER:
			flag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			break;
		case GEOMETRY_SHADER:
			flag = VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case FRAGMENT_SHADER:
			flag = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case COMPUTE_SHADER:
			flag = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case TASK_SHADER_NV:
			flag = VK_SHADER_STAGE_TASK_BIT_NV;
			break;
		case MESH_SHADER_NV:
			flag = VK_SHADER_STAGE_MESH_BIT_NV;
			break;
		case RAY_GENERATION_SHADER_KHR:
			flag = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			break;
		case RAY_HIT_SHADER_KHR:
			flag = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			break;
		case RAY_CLOSEST_HIT_SHADER_KHR:
			flag = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			break;
		case RAY_MISS_SHADER_KHR:
			flag = VK_SHADER_STAGE_MISS_BIT_KHR;
			break;
		case RAY_INTERSECTION_SHADER_KHR:
			flag = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
			break;
		case RAY_CALLABLE_SHADER_KHR:
			flag = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			break;
		default:
			return false;
		}

		CreateStageInfo(info, module, "main", flag);

		modules[counter] = module;
		shaderStages[counter++] = info;

		return true;
	}

	void VKShader::BindSets(CommandBuffer* commandBuffer, uint32_t first, uint32_t set_count, uint32_t instance_index) const
	{
		//if (set >= descriptorPool->getNumberOfSets() || instance_index >= instance_counter)
		//	PR_LOG_RUNTIME_ERROR("Tried to bind set %i, with instance index %i, but only 0 to %i sets and instance indices 0 to %i are valid!\n", set, instance_index, descriptorPool->getNumberOfSets(), instance_counter - 1);

		VKCommandBuffer* vkcmdbuff = reinterpret_cast<VKCommandBuffer*>(commandBuffer);

		std::vector<VkDescriptorSet> sets(set_count);

		uint32_t index = swapchain->getAquiredImageIndex();
		for (uint32_t i = 0; i < set_count; i++)
		{
			sets[i] = descriptorPool->getSet(i + first, instance_index)->getSets()[index];
		}

		vkCmdBindDescriptorSets(vkcmdbuff->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, first, set_count, sets.data(), 0, nullptr);
	}

	void VKShader::SetUniformi(const std::string& name, int value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, sizeof(int));
		buff->Store(&value);
		buff->UnmapMemory();
	}

	void VKShader::SetUniformf(const std::string& name, float value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, sizeof(float));
		buff->Store(&value);
		buff->UnmapMemory();
	}

	void VKShader::SetUniform(const std::string& name, const Vector2f& value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, Vector2f::size());
		buff->Store((void*)&value);
		buff->UnmapMemory();
	}

	void VKShader::SetUniform(const std::string& name, const Vector3f& value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, Vector3f::size());
		buff->Store((void*)&value);
		buff->UnmapMemory();
	}

	void VKShader::SetUniform(const std::string& name, const Vector4f& value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, Vector4f::size());
		buff->Store((void*)&value);
		buff->UnmapMemory();
	}

	void VKShader::SetUniform(const std::string& name, const Matrix4f& value, size_t offset, uint32_t instance_index) const
	{
		VKBuffer* buff = descriptorPool->getUniform(name, instance_index)->getBuffer();
		buff->MapMemory(offset, sizeof(float) * 16);
		buff->Store(value.m);
		buff->UnmapMemory();
	}

	void VKShader::SetTexture(const std::string& name, Texture* value, uint32_t instance_index) const
	{
		VKTexture* tex = (VKTexture*)value;
		descriptorPool->getUniform(name, instance_index)->setTexture(tex);

		std::pair<uint32_t, uint32_t> location = descriptorPool->getUniformLocation(name);

		std::vector<VkWriteDescriptorSet> sets(swapchain->getSwapchainImages().size());

		for (size_t i = 0; i < sets.size(); i++)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = tex->getTextureImageView();
			imageInfo.sampler = tex->getTextureSampler();

			sets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			sets[i].dstSet = descriptorPool->getSet(location.first, instance_index)->getSets()[i];
			sets[i].dstBinding = location.second;
			sets[i].dstArrayElement = 0;
			sets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			sets[i].descriptorCount = 1;
			sets[i].pImageInfo = &imageInfo;
		}

		vkUpdateDescriptorSets(device->getDevice(), (uint32_t)sets.size(), sets.data(), 0, nullptr);
	}

	void VKShader::SetUniform(const std::string& name, const void* value, size_t size, size_t offset, uint32_t instance_index) const
	{
		void* data;
		const VkDeviceMemory& mem = descriptorPool->getUniform(name, instance_index)->getBuffer()->getMemory();

		vkMapMemory(device->getDevice(), mem, offset, size, 0, &data);
		memcpy(data, value, size);
		vkUnmapMemory(device->getDevice(), mem);
	}

	void VKShader::RegisterInstance()
	{
		descriptorPool->registerInstance();
	}

	VkShaderModule VKShader::CreateShaderModule(const std::vector<char>& code) const
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create shader module!\n");
			return VK_NULL_HANDLE;
		}

		return shaderModule;
	}

	void VKShader::CreateStageInfo(VkPipelineShaderStageCreateInfo& info, VkShaderModule module, const char* main, VkShaderStageFlagBits type)
	{
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage = type;
		info.module = module;
		info.pName = main;
	}
};