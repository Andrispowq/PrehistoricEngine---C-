#include "Includes.hpp"
#include "OBJLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "prehistoric/core/model/material/Material.h"

namespace std
{
	using namespace Prehistoric;

	template<>
	struct hash<Vector2f>
	{
		size_t operator()(Vector2f const& vector) const
		{
			return ((hash<float>()(vector.x) ^
				(hash<float>()(vector.y) << 1)));
		}
	};

	template<>
	struct hash<Vector3f>
	{
		size_t operator()(Vector3f const& vector) const
		{
			return ((hash<float>()(vector.x) ^
				(hash<float>()(vector.y) << 1)) >> 1) ^
				(hash<float>()(vector.z) << 1);
		}
	};

	template<>
	struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<Vector3f>()(vertex.position) ^
				(hash<Vector3f>()(vertex.normal) << 1)) >> 1) ^
				(hash<Vector2f>()(vertex.texture) << 1);
		}
	};
}

namespace Prehistoric
{
	Model OBJLoader::LoadModel(const std::string& path, const std::string& objectFile, const std::string& materialFile)
	{
		PR_PROFILE("Prehistoric::OBJLoader::LoadModel(), path: " + path);

		tinyobj::ObjReaderConfig config;
		config.triangulate = true;
		config.mtl_search_path = path.c_str();

		tinyobj::ObjReader reader;

		std::string fullPath = path;
		if (!objectFile.empty())
		{
			fullPath += ('/' + objectFile);
		}

		if (!reader.ParseFromFile(fullPath, config))
		{
			if (!reader.Error().empty()) 
			{
				PR_LOG_RUNTIME_ERROR("TinyOBJLoader error: %s", reader.Error().c_str());
			}

			PR_LOG_RUNTIME_ERROR("TinyOBJLoader ERROR!\n");
		}

		if (!reader.Warning().empty()) 
		{
			PR_LOG_WARNING("TinyObjReader: %s", reader.Warning().c_str());
		}

		tinyobj::attrib_t attrib = reader.GetAttrib();
		std::vector<tinyobj::shape_t> shapes = reader.GetShapes();
		std::vector<tinyobj::material_t> materials = reader.GetMaterials();

		Model model;

		//Add materials
		size_t index = 0;
		for (const auto& material : materials)
		{
			Material* new_mat = new Material(manager);

			if(!material.diffuse_texname.empty()) new_mat->addTexture(ALBEDO_MAP, manager->loadTexture("res/textures/" + material.diffuse_texname, Anisotropic, Repeat).value());
			if(!material.normal_texname.empty()) new_mat->addTexture(NORMAL_MAP, manager->loadTexture("res/textures/" + material.normal_texname, Anisotropic, Repeat).value());

			if(material.diffuse_texname.empty()) new_mat->addVector3f(COLOUR, { material.diffuse[0], material.diffuse[1], material.diffuse[2] });
			new_mat->addVector4f(MROT, { /*material.metallic*/ 0, /*material.roughness*/ 1, 1, -1 });
			new_mat->addFloat(EMISSION, 0.0f);

			model.AddMaterial(new_mat);
		}

		//Add meshes
		for (const auto& shape : shapes) 
		{
			Mesh mesh;

			std::unordered_map<Vertex, uint32_t> uniqueVertices{};
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			int matIdx = shape.mesh.material_ids[0];
			Vector3f average = 0;
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex;

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				average += vertex.position;

				if (index.texcoord_index != -1)
				{
					vertex.texture = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (index.normal_index != -1)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}

			for (size_t i = 0; i < indices.size() / 3; i++)
			{
				uint32_t i0 = indices[i * 3 + 0];
				uint32_t i1 = indices[i * 3 + 1];
				uint32_t i2 = indices[i * 3 + 2];

				Vertex& v0 = vertices[i0];
				Vertex& v1 = vertices[i1];
				Vertex& v2 = vertices[i2];

				CalculateTangents(v0, v1, v2);
			}

			for (auto& vert : vertices)
			{
				vert.averageTangents();
			}

			mesh.setVertices(std::move(vertices));
			mesh.setIndices(std::move(indices));
			mesh.setMaterialIndex(matIdx);

			model.AddMesh(std::move(mesh));
		}

		return model;
	}

	void OBJLoader::CalculateTangents(Vertex& v0, Vertex& v1, Vertex& v2)
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
