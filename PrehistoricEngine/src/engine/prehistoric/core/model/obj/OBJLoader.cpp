#include "Includes.hpp"
#include "OBJLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Prehistoric
{
	namespace OBJLoader
	{
		Mesh OBJLoader::LoadMesh(const std::string& path, const std::string& objectFile, const std::string& materialFile)
		{
			PR_PROFILE("Prehistoric::OBJLoader::LoadMesh(), path: " + path);

			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn, err;

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path + objectFile).c_str())) 
			{
				PR_LOG_ERROR("TinyOBJLoader error: %s\n", (warn + err).c_str());
			}

			std::vector<Vertex> vertices;
			std::vector<uint16_t> indices;

			std::unordered_map<size_t, uint32_t> uniqueVertexIDs{};
			std::unordered_map<size_t, Vertex> uniqueVertices{};

			size_t id = 0;
			for (const auto& shape : shapes) 
			{
				for (const auto& index : shape.mesh.indices) 
				{
					Vertex vertex{};
					vertex.setID(id);

					vertex.setPosition({
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					});

					vertex.setTexture({
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					});

					vertex.setNormal({
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					});

					uint16_t count = 0;
					for (const auto& v : uniqueVertexIDs)
					{
						Vertex vert = uniqueVertices[v.first];

						if (vert.getPosition() == vertex.getPosition() &&
							vert.getTexture() == vertex.getTexture() &&
							vert.getNormal() == vertex.getNormal())
						{
							count++;
						}
					}

					if (count == 0)
					{
						uniqueVertexIDs[id] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertexIDs[id]);
					id++;
				}
			}

			for (int i = 0; i < indices.size() / 3; i++)
			{
				uint16_t i0 = indices[i * 3 + 0];
				uint16_t i1 = indices[i * 3 + 1];
				uint16_t i2 = indices[i * 3 + 2];

				Vertex& v0 = vertices[i0];
				Vertex& v1 = vertices[i1];
				Vertex& v2 = vertices[i2];

				CalculateTangents(v0, v1, v2);
			}

			for (auto& vert : vertices)
			{
				vert.averageTangents();
			}

			Mesh mesh;
			mesh.setVertices(vertices);
			mesh.setIndices(indices);

			return mesh;
		}

		void CalculateTangents(Vertex& v0, Vertex& v1, Vertex& v2)
		{
			Vector3f e1 = v1.getPosition() - v0.getPosition();
			Vector3f e2 = v2.getPosition() - v0.getPosition();

			Vector2f uv0 = v0.getTexture();
			Vector2f uv1 = v1.getTexture();
			Vector2f uv2 = v2.getTexture();

			Vector2f deltaUV1 = uv1 - uv0;
			Vector2f deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			Vector3f tangent = ((e1 * deltaUV2.y - e2 * deltaUV1.y) * r).normalise();

			v0.addTangent(tangent);
			v1.addTangent(tangent);
			v2.addTangent(tangent);
		}
	};
};
