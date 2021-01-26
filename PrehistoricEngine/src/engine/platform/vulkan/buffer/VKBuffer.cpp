#include "Includes.hpp"
#include "VKBuffer.h"

namespace Prehistoric
{
	VKBuffer::VKBuffer(VKDevice* device, size_t size, VkBufferUsageFlags buFlags, VkMemoryPropertyFlags mpFlags)
		: device(device), size(size), buFlags(buFlags), mpFlags(mpFlags)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = buFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device->getDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VKUtil::FindMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, mpFlags);

		if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(device->getDevice(), buffer, memory, 0);
	}

	VKBuffer::~VKBuffer()
	{
		vkDestroyBuffer(device->getDevice(), buffer, nullptr);
		vkFreeMemory(device->getDevice(), memory, nullptr);
	}

	void VKBuffer::MapMemory(size_t offset, size_t size)
	{
		vkMapMemory(device->getDevice(), memory, offset, (size == -1) ? this->size : size, 0, &mapped);
		mapped_size = size;
	}

	void VKBuffer::UnmapMemory()
	{
		mapped_size = -1;
		vkUnmapMemory(device->getDevice(), memory);
	}

	void VKBuffer::Store(void* data)
	{
		memcpy(mapped, data, mapped_size == -1 ? size : mapped_size);
	}

	void VKBuffer::Copy(VKBuffer* src)
	{
		VKUtil::CopyBuffer(device, src->buffer, buffer, size);
	}

	VKBuffer::VKBuffer(const VKBuffer& other)
		: device(other.device), size(other.size), buFlags(other.buFlags), mpFlags(other.mpFlags)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = buFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device->getDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VKUtil::FindMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, mpFlags);

		if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(device->getDevice(), buffer, memory, 0);

		//VKUtil::CopyBuffer(device, other.buffer, buffer, size);
	}

	VKBuffer::VKBuffer(VKBuffer&& other) noexcept
	{
		this->device = other.device;
		this->size = other.size;
		this->buFlags = other.buFlags;
		this->mpFlags = other.mpFlags;

		this->buffer = other.buffer;
		this->memory = other.memory;
		other.buffer = 0;
		other.memory = 0;
	}

	VKBuffer& VKBuffer::operator=(VKBuffer other)
	{
		this->device = other.device;
		this->size = other.size;
		this->buFlags = other.buFlags;
		this->mpFlags = other.mpFlags;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = buFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device->getDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VKUtil::FindMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, mpFlags);

		if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(device->getDevice(), buffer, memory, 0);

		return *this;
	}
};
