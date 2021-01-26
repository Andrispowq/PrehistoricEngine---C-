#ifndef VK_DESCRIPTOR_POOL_H
#define VK_DESCRIPTOR_POOL_H

#include "Includes.hpp"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

#include "VKDescriptorSet.h"

namespace Prehistoric
{
	class VKDescriptorPool
	{
	public:
		VKDescriptorPool(VKDevice* device, VKSwapchain* swapchain);
		virtual ~VKDescriptorPool();

		void addSet(VKDescriptorSet* set);
		void addSet();

		/*
			This method will create the descriptor pool, allocate the sets, and so on, and the only thing you can change now is the new instances
		*/
		void finalise(VkPipelineLayout& layout);

		void registerInstance();

		void addUniform(const std::string& name, uint32_t stages, UniformType type, uint32_t set, uint32_t binding, uint32_t size, Texture* texture);
		VKDescriptorSetBinding* getUniform(const std::string& name, uint32_t instance_index);

		VKDescriptorSet* getSet(uint32_t set_index, uint32_t instance_index) { return sets[size_t(instance_index * numberOfSets + set_index)]; }
		uint32_t getNumberOfSets() const { return numberOfSets; }

		inline VkDescriptorPool& getDescriptorPool() { return pool; }

		inline std::pair<uint32_t, uint32_t> getUniformLocation(const std::string& name) const { return uniformLocations.find(name)->second; }

		VKDescriptorPool(const VKDescriptorPool&) = default;

	private:
		VKDevice* device;
		VKSwapchain* swapchain;

		std::array<VkDescriptorPoolSize, 2> poolSizes;
		VkDescriptorPool pool;

		uint32_t uniformCount;
		uint32_t textureCount;

		uint32_t instance_count = 0;

		uint32_t numberOfSets;
		std::vector<VKDescriptorSet*> sets; //First, we allocate the amount of sets as the swapchain images, then extend the vector when recieving the number of instances
		//Getting set s at swapchain index i -> sets[s * numImages + i]

		std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> uniformLocations; //We store here which uniform name corresponds to which set (std::pair::first) and
		//which binding (std::pair::second)
	};
};

#endif