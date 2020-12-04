#include "engine/prehistoric/core/util/Includes.hpp"
#include "ModelFabricator.h"
#include "engine/platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"

namespace ModelFabricator
{
	MeshVertexBuffer* ModelFabricator::CreateQuad(Window* window)
	{
		MeshVertexBuffer* vbo = nullptr;

		Mesh mesh;

		mesh.AddVertex(Vertex(Vector3f(-1, -1, 0), Vector2f(0, 0), Vector3f(0, 0, -1)));
		mesh.AddVertex(Vertex(Vector3f(-1, 1, 0), Vector2f(0, 1), Vector3f(0, 0, -1)));
		mesh.AddVertex(Vertex(Vector3f(1, 1, 0), Vector2f(1, 1), Vector3f(0, 0, -1)));
		mesh.AddVertex(Vertex(Vector3f(1, -1, 0), Vector2f(1, 0), Vector3f(0, 0, -1)));

		mesh.setIndices({ 0, 1, 2, 2, 3, 0 });

		if (FrameworkConfig::api == OpenGL)
		{
			vbo = new GLMeshVertexBuffer(mesh);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			vbo = new VKMeshVertexBuffer(mesh, window);
		}

		return vbo;
	}
};