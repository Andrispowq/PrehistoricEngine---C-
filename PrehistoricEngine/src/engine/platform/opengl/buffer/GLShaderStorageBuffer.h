#ifndef GL_SSBO_H
#define GL_SSBO_H

#include "engine/prehistoric/common/buffer/ShaderStorageBuffer.h"

#include <glew.h>

namespace Prehistoric
{
	class GLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		GLShaderStorageBuffer(void* data, const Layout& layout);
		virtual ~GLShaderStorageBuffer() override;

		virtual void Bind(CommandBuffer* commandBuffer, uint32_t binding) const override;
		virtual void Unbind() const override;

		virtual void MapBuffer() override;
		virtual void UnmapBuffer() override;
	private:
		GLuint id;
	};
};

#endif