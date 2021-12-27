#ifndef GL_UBO_H
#define GL_UBO_H

#include "prehistoric/common/buffer/UniformBufferObject.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLUniformBufferObject : public UniformBufferObject
	{
	public:
		GLUniformBufferObject(Window* window, void* data, size_t size);
		virtual ~GLUniformBufferObject() override;

		virtual void BindBase(CommandBuffer* commandBuffer, uint32_t binding) const override;
		virtual void UnbindBase(uint32_t binding) const override;

		virtual void Bind(CommandBuffer* commandBuffer) const override;
		virtual void Unbind() const override;

		virtual void MapBuffer() override;
		virtual void UnmapBuffer() override;
	private:
		GLuint id;
	};
};

#endif