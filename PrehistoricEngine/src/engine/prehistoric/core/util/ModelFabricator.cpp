#include "Includes.hpp"
#include "ModelFabricator.h"
#include "platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"

namespace Prehistoric
{
	namespace ModelFabricator
	{
		MeshVertexBuffer* ModelFabricator::CreateQuad(Window* window)
		{
			MeshVertexBuffer* vbo = nullptr;

			Model model;

			Mesh mesh;
			mesh.AddVertex(Vertex(Vector3f(-1, -1, 0), Vector2f(0, 0), Vector3f(0, 0, -1)));
			mesh.AddVertex(Vertex(Vector3f(-1, 1, 0), Vector2f(0, 1), Vector3f(0, 0, -1)));
			mesh.AddVertex(Vertex(Vector3f(1, 1, 0), Vector2f(1, 1), Vector3f(0, 0, -1)));
			mesh.AddVertex(Vertex(Vector3f(1, -1, 0), Vector2f(1, 0), Vector3f(0, 0, -1)));

			mesh.setIndices({ 0, 1, 2, 2, 3, 0 });

			model.AddMesh(std::move(mesh));

			if (FrameworkConfig::api == OpenGL)
			{
				vbo = new GLMeshVertexBuffer(window, model);
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				vbo = new VKMeshVertexBuffer(window, model);
			}

			return vbo;
		}
	};
};
