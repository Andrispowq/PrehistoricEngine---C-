#include "Includes.hpp"
#include "GLComputePipeline.h"

namespace Prehistoric
{
	GLComputePipeline::GLComputePipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: GLPipeline(window, manager, shader), ComputePipeline()
	{
	}

	void GLComputePipeline::BindPipeline(CommandBuffer* buffer) const
	{
		GLPipeline::BindPipeline(buffer);

		for (const auto& binding : textureBindingTable)
		{
			GLenum access;

			if (binding.second.second == READ_ONLY)
				access = GL_READ_ONLY;
			else if (binding.second.second == WRITE_ONLY)
				access = GL_WRITE_ONLY;
			else if (binding.second.second == READ_WRITE)
				access = GL_READ_WRITE;
			else
				access = GL_READ_WRITE;

			//It is very important to cast the texture to the given type, because Texture has also got an ID and a getID method which gives an explicit renderID, not the internal GL ID
			GLenum internalFormat = GLTexture::getFormatForComputeShader(binding.second.first.first->getFormat());

			if(binding.second.first.first->getType() == TEXTURE_CUBE_MAP)
				glBindImageTexture((uint32_t)binding.first, ((GLTexture*)binding.second.first.first)->getTextureID(), (uint32_t)binding.second.first.second, GL_TRUE, 0, access, internalFormat);
			else
				glBindImageTexture((uint32_t)binding.first, ((GLTexture*)binding.second.first.first)->getTextureID(), (uint32_t)binding.second.first.second, GL_FALSE, 0, access, internalFormat);
		}

		for (const auto& ssbo : ssboBindingTable)
		{
			ssbo.second.first->Bind(nullptr, ssbo.first);
		}
	}

	void GLComputePipeline::RenderPipeline() const
	{
		GLPipeline::RenderPipeline();

		glDispatchCompute(invocationSize.x, invocationSize.y, invocationSize.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glFinish();
	}

	void GLComputePipeline::UnbindPipeline() const
	{
		GLPipeline::UnbindPipeline();

		for (const auto& ssbo : ssboBindingTable)
		{
			ssbo.second.first->Unbind();
		}
	}
};