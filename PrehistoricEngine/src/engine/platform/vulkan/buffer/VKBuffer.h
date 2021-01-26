#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric
{
	/*
		Abstracts away a VkBuffer object and it's memory, with keeping track of the size
	*/
	class VKBuffer
	{
	public:
		VKBuffer(VKDevice* device, size_t size, VkBufferUsageFlags buFlags, VkMemoryPropertyFlags mpFlags);
		virtual ~VKBuffer();

		void MapMemory(size_t offset = 0, size_t size = -1);
		void UnmapMemory();

		void Store(void* data);
		void Copy(VKBuffer* src);

		inline VkBuffer& getBuffer() { return buffer; }
		inline VkDeviceMemory& getMemory() { return memory; }

		inline size_t getSize() const { return size; }
		inline void setSize(size_t size) { this->size = size; }

		//We allow for copying and moving in this case because it is meant to be used in a platform-dependant way
		VKBuffer(const VKBuffer& other);
		VKBuffer(VKBuffer&& other) noexcept;
		VKBuffer& operator=(VKBuffer other);
	private:
		VKDevice* device;

		VkBuffer buffer;
		VkDeviceMemory memory;
		size_t size;

		void* mapped;
		size_t mapped_size;

		VkBufferUsageFlags buFlags;
		VkMemoryPropertyFlags mpFlags;
	};
};

#endif