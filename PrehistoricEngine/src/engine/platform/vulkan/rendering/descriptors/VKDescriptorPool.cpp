#include "Includes.hpp"
#include "VKDescriptorPool.h"

namespace Prehistoric
{
	VKDescriptorPool::VKDescriptorPool(VKDevice* device, VKSwapchain* swapchain)
		: device(device), swapchain(swapchain)
	{
	}

	VKDescriptorPool::~VKDescriptorPool()
	{
		for (const auto& set : sets)
		{
			delete set;
		}

		vkDestroyDescriptorPool(device->getDevice(), pool, nullptr);
	}

	void VKDescriptorPool::addSet(VKDescriptorSet* set)
	{
		sets.push_back(set);
	}

	void VKDescriptorPool::addSet()
	{
		sets.push_back(new VKDescriptorSet(device, swapchain, (uint32_t)sets.size()));
	}

	void VKDescriptorPool::finalise(VkPipelineLayout& layout)
	{
		uint32_t numImages = (uint32_t)swapchain->getSwapchainImages().size();

		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkDescriptorSet> _sets;
		layouts.reserve(sets.size());
		_sets.resize(sets.size() * numImages);

		numberOfSets = (uint32_t)sets.size();

		for (auto& set : sets)
		{
			set->finalise();
			layouts.push_back(set->getLayout());
		}

		VkPipelineLayoutCreateInfo _layoutCreateInfo = {};
		_layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		_layoutCreateInfo.setLayoutCount = (uint32_t)layouts.size();
		_layoutCreateInfo.pSetLayouts = layouts.data();
		_layoutCreateInfo.pushConstantRangeCount = 0; // For now
		_layoutCreateInfo.pPushConstantRanges = nullptr; // For now

		if (vkCreatePipelineLayout(device->getDevice(), &_layoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create the pipeline layout in VkDescriptorPool::finalise(void)!\n");
		}

		std::vector<VkDescriptorPoolSize> _poolSizes(2);

		uniformCount = 0;
		textureCount = 0;

		for (auto& set : sets)
		{
			for (auto& binding : set->getBindings())
			{
				if (binding->getBuffer() == nullptr) //It's either a uniform buffer or a texture descriptor now
					textureCount++;
				else
					uniformCount++;
			}
		}

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = uniformCount * numImages;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = textureCount * numImages;

		VkDescriptorPoolCreateInfo _poolCreateInfo = {};
		_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		_poolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
		_poolCreateInfo.pPoolSizes = poolSizes.data();
		_poolCreateInfo.maxSets = (uint32_t)sets.size() * numImages;
		_poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(device->getDevice(), &_poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create the descriptor pool in VkDescriptorPool::finalise(void)!\n");
		}

		std::vector<VkDescriptorSetLayout> _layouts(layouts.size() * numImages);

		for (size_t i = 0; i < layouts.size(); i++)
		{
			for (uint32_t j = 0; j < numImages; j++)
			{
				_layouts[i * numImages + j] = layouts[i];
			}
		}

		VkDescriptorSetAllocateInfo _setAllocInfo = {};
		_setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		_setAllocInfo.descriptorPool = pool;
		_setAllocInfo.descriptorSetCount = (uint32_t)sets.size() * numImages; //We create a set for every swapchain image for easier synchronisation
		_setAllocInfo.pSetLayouts = _layouts.data();
		_setAllocInfo.pNext = nullptr;

		if (vkAllocateDescriptorSets(device->getDevice(), &_setAllocInfo, _sets.data()) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate %i descriptor sets from Descriptor Pool at location %p in function VKDescriptorPool::finalise(void)!\n", _setAllocInfo.descriptorSetCount, &pool);
		}

		for (size_t i = 0; i < sets.size(); i++)
		{
			for (uint32_t im = 0; im < numImages; im++)
			{
				sets[i]->getSets().push_back(_sets[i * numImages + im]);
			}
		}

		for (auto& set : sets)
		{

			for (auto& binding : set->getBindings())
			{
				if (binding->getBuffer() == nullptr)
					continue;

				std::vector<VkWriteDescriptorSet> _writeSets;
				_writeSets.reserve(numImages);

				for (uint32_t i = 0; i < numImages; i++)
				{
					VkDescriptorBufferInfo _bufferInfo = {};
					_bufferInfo.buffer = binding->getBuffer()->getBuffer();
					_bufferInfo.offset = 0;
					_bufferInfo.range = binding->getBuffer()->getSize();

					VkWriteDescriptorSet _writeSet = {};
					_writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					_writeSet.dstSet = set->getSets()[i];
					_writeSet.dstBinding = binding->getBinding().binding;
					_writeSet.dstArrayElement = 0;
					_writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //For now only uniform buffers are written
					_writeSet.descriptorCount = 1;
					_writeSet.pBufferInfo = &_bufferInfo;

					_writeSets.push_back(_writeSet);
				}

				vkUpdateDescriptorSets(device->getDevice(), numImages, _writeSets.data(), 0, nullptr);
			}
		}
	}

	void VKDescriptorPool::registerInstance()
	{
		uint32_t numImages = (uint32_t)swapchain->getSwapchainImages().size();

		instance_count++;

		//If we have enough sets allocated, just return because we already have what we need
		if (instance_count * numImages <= sets.size())
		{
			return;
		}

		//If we need to allocate more sets, do it now 

		//Double the amount of current sets, so we don't need to do this for every added instance
		std::vector<VKDescriptorSet*> added_sets(sets.size());

		for (size_t i = 0; i < sets.size(); i++)
		{
			added_sets[i] = new VKDescriptorSet(*sets[i]);
			added_sets[i]->finalise();
		}

		std::vector<VkDescriptorSetLayout> _layouts(sets.size() * numImages * 2);
		for (size_t x = 0; x < 2; x++)
		{
			for (size_t i = 0; i < sets.size(); i++)
			{
				for (size_t j = 0; j < numImages; j++)
				{
					_layouts[x * (sets.size() * numImages) + i * numImages + j] = sets[i]->getLayout();
				}
			}
		}

		//Recreate the descriptor pool, so we can allocate enough sets
		vkDestroyDescriptorPool(device->getDevice(), pool, nullptr);

		poolSizes[0].descriptorCount *= 2;
		poolSizes[1].descriptorCount *= 2;

		VkDescriptorPoolCreateInfo _poolCreateInfo = {};
		_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		_poolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
		_poolCreateInfo.pPoolSizes = poolSizes.data();
		_poolCreateInfo.maxSets = (uint32_t)sets.size() * numImages * 2;
		_poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(device->getDevice(), &_poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create the descriptor pool in VkDescriptorPool::finalise(void)!\n");
		}

		std::vector<VkDescriptorSet> _sets(sets.size() * numImages * 2);

		//Reallocate all sets, including the previous ones
		VkDescriptorSetAllocateInfo _setAllocInfo = {};
		_setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		_setAllocInfo.descriptorPool = pool;
		_setAllocInfo.descriptorSetCount = (uint32_t)sets.size() * numImages * 2; //We create a set for every swapchain image for easier synchronisation
		_setAllocInfo.pSetLayouts = _layouts.data();
		_setAllocInfo.pNext = nullptr;

		if (vkAllocateDescriptorSets(device->getDevice(), &_setAllocInfo, _sets.data()) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate %i descriptor sets from Descriptor Pool at location %p in function VKDescriptorPool::finalise(void)!\n", _setAllocInfo.descriptorSetCount, &pool);
		}

		for (size_t i = 0; i < added_sets.size(); i++)
		{
			sets.push_back(added_sets[i]);
		}

		for (size_t i = 0; i < sets.size(); i++)
		{
			if (i < sets.size() / 2)
			{
				sets[i]->getSets().clear();
			}

			for (uint32_t im = 0; im < numImages; im++)
			{
				sets[i]->getSets().push_back(_sets[i * numImages + im]);
			}
		}

		for (auto& set : sets)
		{
			for (auto& binding : set->getBindings())
			{
				if (binding->getBuffer() == nullptr)
					continue;

				std::vector<VkWriteDescriptorSet> _writeSets;
				_writeSets.reserve(numImages);

				for (uint32_t i = 0; i < numImages; i++)
				{
					VkDescriptorBufferInfo _bufferInfo = {};
					_bufferInfo.buffer = binding->getBuffer()->getBuffer();
					_bufferInfo.offset = 0;
					_bufferInfo.range = binding->getBuffer()->getSize();

					VkWriteDescriptorSet _writeSet = {};
					_writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					_writeSet.dstSet = set->getSets()[i];
					_writeSet.dstBinding = binding->getBinding().binding;
					_writeSet.dstArrayElement = 0;
					_writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //For now only uniform buffers are written
					_writeSet.descriptorCount = 1;
					_writeSet.pBufferInfo = &_bufferInfo;

					_writeSets.push_back(_writeSet);
				}

				vkUpdateDescriptorSets(device->getDevice(), numImages, _writeSets.data(), 0, nullptr);
			}
		}
	}

	void VKDescriptorPool::addUniform(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, uint32_t size, Texture* texture)
	{
		VKDescriptorSetBinding* _binding = new VKDescriptorSetBinding(type, binding, stages);

		//Textures don't need to be pre-set, so they can be changed at runtime
		/*if (size == 0 && texture == nullptr)
		{
			PR_LOG_RUNTIME_ERROR("Tried to add a uniform binding which is not a buffer nor a texture! No other uniforms are supported now!\n");
		}*/

		if (size == 0)
		{
			_binding->setTexture((VKTexture*)texture);
		}
		else
		{
			VKBuffer* buffer = new VKBuffer(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			_binding->setBuffer(buffer);
		}

		if (sets.size() == set)
		{
			VKDescriptorSet* _set = new VKDescriptorSet(device, swapchain, set);
			_set->addBinding(_binding);
			sets.push_back(_set);
		}
		else if (sets.size() > set)
		{
			sets[set]->addBinding(_binding);
		}
		else
		{
			PR_LOG_RUNTIME_ERROR("Please add the descriptor sets in sequential order!\n");
		}

		uniformLocations.insert(std::make_pair(name, std::make_pair(set, binding)));
	}

	VKDescriptorSetBinding* VKDescriptorPool::getUniform(const std::string& name, uint32_t instance_index)
	{
		auto setBinding = uniformLocations.find(name);
		if (setBinding == uniformLocations.end())
		{
			PR_LOG_RUNTIME_ERROR("Tried accessing uniform %s but it doesn't exist!\n", name.c_str());
		}

		std::pair<uint32_t, uint32_t> _setBinding = setBinding->second;

		return sets[size_t(instance_index * numberOfSets) + _setBinding.first]->getBindings()[_setBinding.second];
	}
};
