#ifndef MESH_H
#define MESH_H

#include "Vertex.h"
#include "obj/Shape.h"

namespace Prehistoric
{
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, 
			const std::vector<Shape>& shapes = {}) : vertices(vertices), indices(indices), shapes(shapes) {}
		Mesh() {}

		~Mesh() {}

		//They return a pointer as returning a vector would result in a huge copy, but they should get deleted later on
		std::vector<float> GetVertexData() const;
		std::vector<uint16_t> GetIndexData() const;

		void AddVertex(const Vertex& vertex) { vertices.push_back(vertex); }
		void AddIndex(const int& index) { indices.push_back(index); }

		inline std::vector<Vertex> getVertices() const { return vertices; }
		inline std::vector<uint16_t> getIndices() const { return indices; }
		inline std::vector<Shape> getShapes() const { return shapes; }

		inline void setVertices(const std::vector<Vertex>& vertices) { this->vertices = vertices; }
		inline void setIndices(const std::vector<uint16_t>& indices) { this->indices = indices; }
		inline void setShapes(const std::vector<Shape>& shapes) { this->shapes = shapes; }

		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh& mesh) = default;
		Mesh& operator=(Mesh&& mesh) = default;
	private:
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

		std::vector<Shape> shapes;
	};
};

#endif