#ifndef GL_MESH_VERTEX_BUFFER_H
#define GL_MESH_VERTEX_BUFFER_H

#include <glew.h>

#include "engine/prehistoric/common/buffer/MeshVertexBuffer.h"

class GLMeshVertexBuffer : public MeshVertexBuffer
{
public:
	GLMeshVertexBuffer(const Mesh& mesh);
	virtual ~GLMeshVertexBuffer() override;

	void Bind(CommandBuffer* commandBuffer) const override;
	void Draw(CommandBuffer* commandBuffer) const override;
	void Unbind() const override;
private:
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
};

#endif