#include "Includes.hpp"
#include "Mesh.h"

namespace Prehistoric
{
	Mesh::Mesh(const Mesh& mesh)
	{
		this->vertices = mesh.vertices;
		this->indices = mesh.indices;
	}

	Mesh::~Mesh()
	{
		vertices.clear();
		indices.clear();
	}

	std::vector<float> Mesh::GetVertexData() const
	{
		constexpr size_t numFloats = Vertex::getNumberOfFloats();
		std::vector<float> data(vertices.size() * numFloats);

		int counter = 0;
		for (size_t i = 0; i < vertices.size(); i++)
		{
			counter = 0;

			data[i * numFloats + counter++] = vertices[i].getPosition().x;
			data[i * numFloats + counter++] = vertices[i].getPosition().y;
			data[i * numFloats + counter++] = vertices[i].getPosition().z;
			data[i * numFloats + counter++] = vertices[i].getTexture().x;
			data[i * numFloats + counter++] = vertices[i].getTexture().y;
			data[i * numFloats + counter++] = vertices[i].getNormal().x;
			data[i * numFloats + counter++] = vertices[i].getNormal().y;
			data[i * numFloats + counter++] = vertices[i].getNormal().z;
			data[i * numFloats + counter++] = vertices[i].getTangent().x;
			data[i * numFloats + counter++] = vertices[i].getTangent().y;
			data[i * numFloats + counter++] = vertices[i].getTangent().z;
		}

		return data;
	}

	std::vector<uint16_t> Mesh::GetIndexData() const
	{
		return indices;
	}
};
