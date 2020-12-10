#include "Includes.hpp"
#include "VKBuffer.h"

namespace Prehistoric
{
	VKBuffer::VKBuffer(VKPhysicalDevice* physicalDevice, VKDevice* device, size_t size, VkBufferUsageFlags buFlags, VkMemoryPropertyFlags mpFlags)
		: physicalDevice(physicalDevice), device(device), size(size), buFlags(buFlags), mpFlags(mpFlags)
	{
		VKUtil::CreateBuffer(physicalDevice->getPhysicalDevice(), device->getDevice(), size, buFlags, mpFlags, buffer, memory);
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
		: physicalDevice(other.physicalDevice), device(other.device), size(other.size), buFlags(other.buFlags), mpFlags(other.mpFlags)
	{
		VKUtil::CreateBuffer(physicalDevice->getPhysicalDevice(), device->getDevice(), size, buFlags, mpFlags, buffer, memory);
		//VKUtil::CopyBuffer(device, other.buffer, buffer, size);
	}

	VKBuffer::VKBuffer(VKBuffer&& other) noexcept
	{
		this->physicalDevice = other.physicalDevice;
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
		this->physicalDevice = other.physicalDevice;
		this->device = other.device;
		this->size = other.size;
		this->buFlags = other.buFlags;
		this->mpFlags = other.mpFlags;

		VKUtil::CreateBuffer(physicalDevice->getPhysicalDevice(), device->getDevice(), size, buFlags, mpFlags, buffer, memory);
		return *this;
	}
};
