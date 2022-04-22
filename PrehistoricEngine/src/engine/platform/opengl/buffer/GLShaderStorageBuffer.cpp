#include "Includes.hpp"
#include "GLShaderStorageBuffer.h"

namespace Prehistoric
{
	/*
		Please note that this function is extremely dangerous, it does not have any type checks, memory bound checks, or whatever, so you need to specify the
		layout CORRECTLY, so it will not cause a crash or something even worse

		'data' gets managed by OpenGL
	*/
	GLShaderStorageBuffer::GLShaderStorageBuffer(Window* window, void* data, size_t size)
		: ShaderStorageBuffer(window, data, size)
	{
		glGenBuffers(1, &id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	GLShaderStorageBuffer::~GLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &id);
	}

	void GLShaderStorageBuffer::BindBase(CommandBuffer* commandBuffer, uint32_t binding) const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
	}

	void GLShaderStorageBuffer::UnbindBase(uint32_t binding) const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0);
	}

	void GLShaderStorageBuffer::Bind(CommandBuffer* commandBuffer) const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void GLShaderStorageBuffer::Unbind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void GLShaderStorageBuffer::MapBuffer()
	{
		data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	}

	void GLShaderStorageBuffer::UnmapBuffer()
	{
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
};
