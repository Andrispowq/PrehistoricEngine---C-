#include "Includes.hpp"
#include "GLUniformBufferObject.h"

namespace Prehistoric
{
	/*
		Please note that this function is extremely dangerous, it does not have any type checks, memory bound checks, or whatever, so you need to specify the
		layout CORRECTLY, so it will not cause a crash or something even worse

		'data' gets managed by OpenGL
	*/
	GLUniformBufferObject::GLUniformBufferObject(Window* window, void* data, size_t size)
		: UniformBufferObject(window, data, size)
	{
		glGenBuffers(1, &id);
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	GLUniformBufferObject::~GLUniformBufferObject()
	{
		glDeleteBuffers(1, &id);
	}

	void GLUniformBufferObject::BindBase(CommandBuffer* commandBuffer, uint32_t binding) const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);
	}

	void GLUniformBufferObject::UnbindBase(uint32_t binding) const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0);
	}

	void GLUniformBufferObject::Bind(CommandBuffer* commandBuffer) const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, id);
	}

	void GLUniformBufferObject::Unbind() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void GLUniformBufferObject::MapBuffer()
	{
		data = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE);
	}

	void GLUniformBufferObject::UnmapBuffer()
	{
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
};
