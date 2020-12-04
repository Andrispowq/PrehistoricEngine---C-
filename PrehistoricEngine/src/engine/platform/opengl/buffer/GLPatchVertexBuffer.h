#ifndef GL_PATCH_VERTEX_BUFFER_H
#define GL_PATCH_VERTEX_BUFFER_H

#include <glew.h>

#include "engine/prehistoric/common/buffer/PatchVertexBuffer.h"

class GLPatchVertexBuffer : public PatchVertexBuffer
{
public:
	GLPatchVertexBuffer(const std::vector<Vector2f>& vertices);
	virtual ~GLPatchVertexBuffer() override;

	void Bind(CommandBuffer* commandBuffer) const override;
	void Draw(CommandBuffer* commandBuffer) const override;
	void Unbind() const override;
private:
	GLuint vao;
	GLuint vbo;
};

#endif