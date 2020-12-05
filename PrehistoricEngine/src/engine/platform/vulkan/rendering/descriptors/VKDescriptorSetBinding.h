#ifndef VK_DESCRIPTOR_SET_BINDING_H
#define VK_DESCRIPTOR_SET_BINDING_H

#include <vulkan/vulkan.h>

#include "prehistoric/common/rendering/shaders/Shader.h"

#include "platform/vulkan/buffer/VKBuffer.h"
#include "platform/vulkan/texture/VkTexture.h"

namespace Prehistoric
{
	class VKDescriptorSetBinding
	{
	public:
		VKDescriptorSetBinding(UniformType type, uint32_t binding, uint32_t stageFlags) : _binding(VkDescriptorSetLayoutBinding{}), type(type), binding(binding), stageFlags(stageFlags) {}
		VKDescriptorSetBinding(const VKDescriptorSetBinding& other);
		virtual ~VKDescriptorSetBinding();

		void finalise();

		inline VkDescriptorSetLayoutBinding& getBinding() { return _binding; }

		inline void setBuffer(VKBuffer* buffer) { if (texture == nullptr) this->buffer = buffer; }
		inline void setTexture(VKTexture* texture) { if (buffer == nullptr) this->texture = texture; }

		inline VKBuffer* getBuffer() const { return buffer; }
		inline VKTexture* getTexture() const { return texture; }
	private:
		VkDescriptorType GetDescriptorType(UniformType type) const;
		uint32_t GetShaderStages(uint32_t stages) const;

		VkDescriptorSetLayoutBinding _binding;
		VKBuffer* buffer = nullptr;
		VKTexture* texture = nullptr;

		bool finalised = false;

		UniformType type;
		uint32_t binding;
		uint32_t stageFlags;
	};
};

#endif