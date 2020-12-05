#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKDescriptorSet.h"

namespace Prehistoric
{
	VKDescriptorSet::VKDescriptorSet(const VKDescriptorSet& other)
		: device(other.device), swapchain(other.swapchain), set_index(other.set_index)
	{
		bindings.reserve(other.bindings.size());

		for (VKDescriptorSetBinding* binding : other.bindings)
		{
			bindings.push_back(new VKDescriptorSetBinding(*binding));
		}
	}

	VKDescriptorSet::~VKDescriptorSet()
	{
		for (const auto& binding : bindings)
		{
			delete binding;
		}

		vkDestroyDescriptorSetLayout(device->getDevice(), layout, nullptr);
	}

	void VKDescriptorSet::addBinding(VKDescriptorSetBinding* binding)
	{
		bindings.push_back(binding);
	}

	void VKDescriptorSet::finalise()
	{
		std::vector<VkDescriptorSetLayoutBinding> _bindings;
		_bindings.reserve(bindings.size());

		for (auto& binding : bindings)
		{
			binding->finalise();
			_bindings.push_back(binding->getBinding());
		}

		VkDescriptorSetLayoutCreateInfo _createInfo = {};
		_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		_createInfo.bindingCount = (uint32_t)_bindings.size();
		_createInfo.pBindings = _bindings.data();

		if (vkCreateDescriptorSetLayout(device->getDevice(), &_createInfo, nullptr, &layout) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("VKDescriptorSet couldn't create a VkDescriptorSetLayout object!\n");
		}
	}
};
