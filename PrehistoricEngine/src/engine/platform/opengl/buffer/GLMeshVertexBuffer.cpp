#include "Includes.hpp"
#include "GLMeshVertexBuffer.h"

#include "prehistoric/core/engines/RenderingEngine.h"

namespace Prehistoric
{
	GLMeshVertexBuffer::GLMeshVertexBuffer(Window* window, const Mesh& mesh)
		: MeshVertexBuffer(window, mesh)
	{
		size = static_cast<uint32_t>(mesh.getIndices().size());

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ibo);

		glBindVertexArray(vao);

		std::vector<float> vdata = mesh.GetVertexData();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh.getVertices().size() * Vertex::getSize(), vdata.data(), GL_STATIC_DRAW);

		std::vector<uint32_t> idata = mesh.GetIndexData();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.getIndices().size() * sizeof(uint32_t), idata.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(4);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, int(sizeof(float) * Vertex::getNumberOfFloats()), (void*)(sizeof(float) * 0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, int(sizeof(float) * Vertex::getNumberOfFloats()), (void*)(sizeof(float) * 3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, int(sizeof(float) * Vertex::getNumberOfFloats()), (void*)(sizeof(float) * 5));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, int(sizeof(float) * Vertex::getNumberOfFloats()), (void*)(sizeof(float) * 8));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	GLMeshVertexBuffer::~GLMeshVertexBuffer()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
	}

	void GLMeshVertexBuffer::Bind(CommandBuffer* commandBuffer) const
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	}

	void GLMeshVertexBuffer::Draw(CommandBuffer* commandBuffer) const
	{
		RenderingEngine::getStats().drawcalls += 1;
		RenderingEngine::getStats().vertexCount += vertices;
		RenderingEngine::getStats().indexCount += indices;

		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, (void*)0);
	}

	void GLMeshVertexBuffer::Unbind() const
	{
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};