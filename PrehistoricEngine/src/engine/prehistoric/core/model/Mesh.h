#ifndef MESH_H
#define MESH_H

#include "Vertex.h"

namespace Prehistoric
{
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, int materialIndex = -1)
			: vertices(vertices), indices(indices), materialIndex(materialIndex) {}
		Mesh() {}

		~Mesh() {}

		//They return a pointer as returning a vector would result in a huge copy, but they should get deleted later on
		std::vector<float> GetVertexData() const;
		std::vector<uint32_t> GetIndexData() const;

		void AddVertex(const Vertex& vertex) { vertices.push_back(vertex); }
		void AddIndex(const int& index) { indices.push_back(index); }

		inline std::vector<Vertex> getVertices() const { return vertices; }
		inline std::vector<uint32_t> getIndices() const { return indices; }
		inline int getMaterialIndex() const { return materialIndex; }

		inline void setVertices(std::vector<Vertex>&& vertices) { this->vertices = vertices; }
		inline void setIndices(std::vector<uint32_t>&& indices) { this->indices = indices; }
		inline void setMaterialIndex(int materialIndex) { this->materialIndex = materialIndex; }

		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh& mesh) = default;
		Mesh& operator=(Mesh&& mesh) = default;
	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		int materialIndex;
	};
};

#endif