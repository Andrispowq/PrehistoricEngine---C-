#include "Includes.hpp"
#include "VKDescriptorSetBinding.h"

namespace Prehistoric
{
	VKDescriptorSetBinding::VKDescriptorSetBinding(const VKDescriptorSetBinding& other)
		: _binding(other._binding), type(other.type), binding(other.binding), stageFlags(other.stageFlags)
	{
		if (other.buffer != nullptr)
		{
			buffer = new VKBuffer(*other.buffer);
		}
	}

	VKDescriptorSetBinding::~VKDescriptorSetBinding()
	{
		if (!finalised)
			return;

		if (texture == nullptr)
		{
			if (buffer == nullptr)
			{
				return;
			}
			else
			{
				delete buffer;
			}
		}
	}

	void VKDescriptorSetBinding::finalise()
	{
		_binding.binding = binding;
		_binding.descriptorCount = 1;
		_binding.descriptorType = GetDescriptorType(type);
		_binding.stageFlags = stageFlags;
		_binding.pImmutableSamplers = nullptr; //For now

		finalised = true;
	}

	VkDescriptorType VKDescriptorSetBinding::GetDescriptorType(UniformType type) const
	{
		VkDescriptorType bindingType;

		if (type == Sampler) bindingType = VK_DESCRIPTOR_TYPE_SAMPLER;
		if (type == CombinedImageSampler) bindingType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		if (type == SampledImage) bindingType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		if (type == StorageImage) bindingType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		if (type == UniformTexelBuffer) bindingType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		if (type == StorageTexelBuffer) bindingType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		if (type == UniformBuffer) bindingType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		if (type == StorageBuffer) bindingType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		if (type == UniformBufferDynamic) bindingType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		if (type == StorageBufferDynamic) bindingType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		if (type == InputAttachment) bindingType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		if (type == InlineUniformBlock_EXT) bindingType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
		if (type == AccelerationStructure_KHR) bindingType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		return bindingType;
	}

	uint32_t VKDescriptorSetBinding::GetShaderStages(uint32_t stages) const
	{
		uint32_t flags = 0;

		if (stages & VERTEX_SHADER) flags = flags | VK_SHADER_STAGE_VERTEX_BIT;
		if (stages & TESSELLATION_CONTROL_SHADER) flags = flags | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if (stages & TESSELLATION_EVALUATION_SHADER) flags = flags | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if (stages & GEOMETRY_SHADER) flags = flags | VK_SHADER_STAGE_GEOMETRY_BIT;
		if (stages & FRAGMENT_SHADER) flags = flags | VK_SHADER_STAGE_FRAGMENT_BIT;
		if (stages & RAY_GENERATION_SHADER_KHR) flags = flags | VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (stages & RAY_HIT_SHADER_KHR) flags = flags | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		if (stages & RAY_CLOSEST_HIT_SHADER_KHR) flags = flags | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		if (stages & RAY_CALLABLE_SHADER_KHR) flags = flags | VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		if (stages & RAY_MISS_SHADER_KHR) flags = flags | VK_SHADER_STAGE_MISS_BIT_KHR;
		if (stages & RAY_INTERSECTION_SHADER_KHR) flags = flags | VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		if (stages & TASK_SHADER_NV) flags = flags | VK_SHADER_STAGE_TASK_BIT_NV;
		if (stages & MESH_SHADER_NV) flags = flags | VK_SHADER_STAGE_MESH_BIT_NV;

		return flags;
	}
};
