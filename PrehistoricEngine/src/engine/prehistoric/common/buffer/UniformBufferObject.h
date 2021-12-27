#ifndef UBO_H
#define UBO_H

#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/rendering/command/CommandBuffer.h"

namespace Prehistoric
{
	class UniformBufferObject
	{
	public:
		UniformBufferObject(Window* window, void* data, size_t size) : window(window), data(data), size(size) {}
		virtual ~UniformBufferObject() = 0;

		virtual void BindBase(CommandBuffer* commandBuffer, uint32_t binding) const = 0;
		virtual void UnbindBase(uint32_t binding) const = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const = 0;
		virtual void Unbind() const = 0;

		virtual void MapBuffer() = 0;
		virtual void UnmapBuffer() = 0;

		void* getMappedData() const { return data; }
	protected:
		Window* window;

		void* data;
		size_t size;
	};
};

#endif