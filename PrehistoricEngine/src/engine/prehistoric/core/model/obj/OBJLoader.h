#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "engine/prehistoric/common/buffer/MeshVertexBuffer.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"

#include "engine/prehistoric/core/util/Constants.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	namespace OBJLoader
	{
		MeshVertexBuffer* LoadModel(const std::string& path, const std::string& objectFile, const std::string& materialFile, Window* window);
		Mesh LoadMesh(const std::string& path, const std::string& objectFile, const std::string& materialFile, Window* window);

		size_t ProcessVertex(const std::vector<std::string>& vertex, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
		void CalculateTangents(size_t v0i, size_t v1i, size_t v2i, std::vector<Vertex>& vertices, const std::vector<Vector2f>& textureCoords);
		void SetVertexData(std::vector<Vertex>& vertices, const std::vector<Vector2f>& textureCoords, const std::vector<Vector3f>& normals);
		void ConvertDataToArrays(const std::vector<Vertex>& vertices, const std::vector<Vector2f>& textures, const std::vector<Vector3f>& normals,
			float* verticesArray, float* texturesArray, float* normalsArray);
		void ConvertIndicesToArray(const std::vector<uint16_t> indices, uint16_t* indicesArray);
		size_t DealWithAlreadyProcessVertex(Vertex* previousVertex, const int& newTextureIndex, const int& newNormalIndex,
			std::vector<uint16_t>& indices, std::vector<Vertex>& vertices);
	};
};

#endif