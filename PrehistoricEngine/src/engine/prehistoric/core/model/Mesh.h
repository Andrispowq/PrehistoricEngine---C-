#ifndef MESH_H
#define MESH_H

#include "Vertex.h"

#include <memory>

namespace Prehistoric
{
	class Mesh
	{
	public:
		Mesh(const Mesh& mesh);

		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) : vertices(vertices), indices(indices) {}
		Mesh() {}

		virtual ~Mesh();

		//They return a pointer as returning a vector would result in a huge copy, but they should get deleted later on
		std::vector<float> GetVertexData() const;
		std::vector<uint16_t> GetIndexData() const;

		void AddVertex(const Vertex& vertex) { vertices.push_back(vertex); }
		void AddIndex(const int& index) { indices.push_back(index); }

		inline std::vector<Vertex> getVertices() const { return vertices; }
		inline std::vector<uint16_t> getIndices() const { return indices; }

		inline void setVertices(const std::vector<Vertex>& vertices) { this->vertices = vertices; }
		inline void setIndices(const std::vector<uint16_t>& indices) { this->indices = indices; }

		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh& mesh) = default;
	private:
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
	};
};

#endif