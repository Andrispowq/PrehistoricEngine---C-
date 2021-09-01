#include "Includes.hpp"
#include "GLMeshVertexBuffer.h"

#include "prehistoric/core/engines/RenderingEngine.h"

namespace Prehistoric
{
	GLMeshVertexBuffer::GLMeshVertexBuffer(Window* window, const Model& model)
		: MeshVertexBuffer(window, model)
	{
		std::vector<float> vdata;
		std::vector<uint32_t> idata;

		vertices.resize(submeshCount);
		index_offsets.resize(submeshCount);
		sizes.resize(submeshCount);

		size_t vertCount = 0;
		size_t index = 0;
		for (auto& mesh : model.getMeshes())
		{
			std::vector<float> vdat = mesh.GetVertexData();
			std::vector<uint32_t> idat = mesh.GetIndexData();

			for (uint32_t& elem : idat)
			{
				elem += size;
			}

			vdata.insert(vdata.begin(), vdat.begin(), vdat.end());
			idata.insert(idata.begin(), idat.begin(), idat.end());

			size_t verts = mesh.getVertices().size();
			size_t inds = mesh.getIndices().size();

			vertices[index] = verts;
			index_offsets[index] = size;
			sizes[index] = inds;

			size += (uint32_t)inds;
			vertCount += verts;
			index++;
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ibo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertCount * Vertex::getSize(), vdata.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), idata.data(), GL_STATIC_DRAW);

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

	void GLMeshVertexBuffer::Draw(CommandBuffer* commandBuffer, uint32_t submesh) const
	{
		if (submesh >= submeshCount)
		{
			return;
		}

		RenderingEngine::getStats().drawcalls += 1;
		RenderingEngine::getStats().vertexCount += vertices[submesh];
		RenderingEngine::getStats().indexCount += sizes[submesh];

		glDrawElements(GL_TRIANGLES, sizes[submesh], GL_UNSIGNED_INT, (const void*)(index_offsets[submesh] * sizeof(uint32_t)));
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