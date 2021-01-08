#ifndef GL_MESH_VERTEX_BUFFER_H
#define GL_MESH_VERTEX_BUFFER_H

#include "prehistoric/common/buffer/MeshVertexBuffer.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLMeshVertexBuffer : public MeshVertexBuffer
	{
	public:
		GLMeshVertexBuffer(Window* window, const Mesh& mesh);
		virtual ~GLMeshVertexBuffer() override;

		void Bind(CommandBuffer* commandBuffer) const override;
		void Draw(CommandBuffer* commandBuffer) const override;
		void Unbind() const override;
	private:
		GLuint vao;
		GLuint vbo;
		GLuint ibo;
	};
};

#endif