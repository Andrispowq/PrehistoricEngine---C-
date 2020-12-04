#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLPatchVertexBuffer.h"

GLPatchVertexBuffer::GLPatchVertexBuffer(const std::vector<Vector2f>& vertices)
	: PatchVertexBuffer(vertices)
{
	size = (uint32_t)vertices.size();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	std::vector<float> vdata(vertices.size() * 2);

	for (size_t i = 0; i < vertices.size(); i++)
	{
		vdata[i * 2 + 0] = vertices[i].x;
		vdata[i * 2 + 1] = vertices[i].y;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * Vector2f::size(), vdata.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glPatchParameteri(GL_PATCH_VERTICES, size);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLPatchVertexBuffer::~GLPatchVertexBuffer()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void GLPatchVertexBuffer::Bind(CommandBuffer* commandBuffer) const
{
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
}

void GLPatchVertexBuffer::Draw(CommandBuffer* commandBuffer) const
{
	glDrawArrays(GL_PATCHES, 0, size);
}

void GLPatchVertexBuffer::Unbind() const
{
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
}