#include "engine/prehistoric/core/util/Includes.hpp"
#include "OBJLoader.h"

namespace OBJLoader
{
	MeshVertexBuffer* OBJLoader::LoadModel(const std::string& path, const std::string& objectFile, const std::string& materialFile, Window* window)
	{
		Mesh mesh = std::move(LoadMesh(path, objectFile, materialFile, window));
		MeshVertexBuffer* vertexBuffer = nullptr;

		if (FrameworkConfig::api == OpenGL)
		{
			vertexBuffer = new GLMeshVertexBuffer(mesh);
		}
		else
		{
			vertexBuffer = new VKMeshVertexBuffer(mesh, window);
		}

		return vertexBuffer;
	}

	Mesh OBJLoader::LoadMesh(const std::string& path, const std::string& objectFile, const std::string& materialFile, Window* window)
	{
		double start = Time::getTime();

		std::ifstream obj;
		//std::ifstream mtl;

		//if (materialFile != "")
		//	mtl.open((path + materialFile).c_str());

		std::string line;

		//Loading material file
		/*std::string currentMtl = "";

		std::unordered_map<std::string, Material*> materials;

		if (mtl.is_open())
		{
			while (mtl.good())
			{
				std::getline(mtl, line);
				std::vector<std::string> tokens = Util::RemoveEmptyStrings(Util::Split(line, ' '));

				if (tokens.size() == 0)
					continue;

				if (tokens[0] == "newmtl")
				{
					Material* material = new Material(manager, window);
					materials.emplace(tokens[1], material);
					currentMtl = tokens[1];
				}

				if (tokens[0] == "map_Kd")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(ALBEDO_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_bump")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(NORMAL_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_Km")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(METALLIC_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_Kr")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(ROUGHNESS_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_Ko")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(OCCLUSION_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_Ke")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(EMISSION_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "map_Ka")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addTexture(ROUGHNESS_MAP, manager->addTexture(TextureLoader::LoadTexture(path + "/" + tokens[2], window)));
					}
				}
				if (tokens[0] == "Kd")
				{
					Vector3f color = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					materials[currentMtl]->addVector3f(COLOUR, color);
				}
				if (tokens[0] == "Km")
				{
					materials[currentMtl]->addFloat(METALLIC, (float)std::atof(tokens[1].c_str()));
				}
				if (tokens[0] == "Kr")
				{
					materials[currentMtl]->addFloat(ROUGHNESS, (float)std::atof(tokens[1].c_str()));
				}
				if (tokens[0] == "Ko")
				{
					materials[currentMtl]->addFloat(OCCLUSION, (float)std::atof(tokens[1].c_str()));
				}
				if (tokens[0] == "Ke")
				{
					Vector3f emission = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					materials[currentMtl]->addVector3f(EMISSION, emission);
				}
				if (tokens[0] == "Ka")
				{
					Vector3f alpha = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					materials[currentMtl]->addVector3f(ALPHA, alpha);
				}
				if (tokens[0] == "illum")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addFloat("illumiation", (float)std::atof(tokens[1].c_str()));
					}
				}
				if (tokens[0] == "Ns")
				{
					if (tokens.size() > 1)
					{
						materials[currentMtl]->addFloat("shininess", (float)std::atof(tokens[1].c_str()));
					}
				}
			}
		}
		else if (materialFile != "")
		{
			PR_LOG_ERROR("Material file %s couldn't be opened!\n", (path + materialFile).c_str());
		}*/

		obj.open(path + objectFile);

		std::vector<Vertex> vertices;
		std::vector<Vector2f> textures;
		std::vector<Vector3f> normals;
		std::vector<uint16_t> indices;

		bool tex_coords = false;

		if (obj.is_open())
		{
			while (obj.good())
			{
				std::getline(obj, line);
				std::vector<std::string> tokens = Util::RemoveEmptyStrings(Util::Split(line, ' '));

				if (tokens.size() == 0)
					continue;

				if (tokens[0] == "v")
				{
					Vector3f vertexPos((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					Vertex vertex(vertexPos);
					vertices.emplace_back(vertexPos);
				}
				else if (tokens[0] == "vt")
				{
					if (!tex_coords)
						tex_coords = true;

					textures.emplace_back((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()));
				}
				else if (tokens[0] == "vn")
				{
					normals.emplace_back((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
				}
				else if (tokens[0] == "f")
				{
					std::vector<std::string> vertex0 = Util::Split(tokens[1], '/');
					std::vector<std::string> vertex1 = Util::Split(tokens[2], '/');
					std::vector<std::string> vertex2 = Util::Split(tokens[3], '/');

					size_t v0 = ProcessVertex(vertex0, vertices, indices);
					size_t v1 = ProcessVertex(vertex1, vertices, indices);
					size_t v2 = ProcessVertex(vertex2, vertices, indices);
					
					if(tex_coords)
						CalculateTangents(v0, v1, v2, vertices, textures);

					if (tokens.size() > 4)
					{
						std::vector<std::string> vertex3 = Util::Split(tokens[4], '/');
						v0 = ProcessVertex(vertex2, vertices, indices);
						v1 = ProcessVertex(vertex3, vertices, indices);
						v2 = ProcessVertex(vertex0, vertices, indices);

						if(tex_coords)
							CalculateTangents(v0, v1, v2, vertices, textures);
					}
				}
			}

			obj.close();
		}
		else
		{
			PR_LOG_ERROR("Object file %s couldn't be opened!\n", (path + objectFile).c_str());
		}

		for (Vertex& vertex : vertices)
		{
			vertex.averageTangents();
		}

		SetVertexData(vertices, textures, normals);

		PR_LOG_MESSAGE("Time taken to load %s: %fs\n", objectFile.c_str(), (Time::getTime() - start));

		return { vertices, indices };
	}

	size_t OBJLoader::ProcessVertex(const std::vector<std::string>& vertex, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
	{
		//Vertex is always specified
		int index = std::atoi(vertex[0].c_str()) - 1;
		Vertex& currentVertex = vertices[index];
		currentVertex.setIndex(index);

		int textureIndex = -1;
		int normalIndex = -1;

		//There's more data specified then just the vertex position
		if (vertex.size() > 1)
		{
			//There's texture coordinate
			if (vertex[1] != "")
			{
				textureIndex = std::atoi(vertex[1].c_str()) - 1;
			}

			//There's normal vector
			if (vertex[2] != "" && vertex.size() > 2)
			{
				normalIndex = std::atoi(vertex[2].c_str()) - 1;
			}
		}

		if (!currentVertex.isSet())
		{
			currentVertex.setTextureIndex(textureIndex);
			currentVertex.setNormalIndex(normalIndex);

			indices.push_back((uint16_t) index);

			return index;
		}
		else
		{
			return DealWithAlreadyProcessVertex(&currentVertex, textureIndex, normalIndex, indices, vertices);
		}
	}

	void CalculateTangents(size_t v0i, size_t v1i, size_t v2i, std::vector<Vertex>& vertices, const std::vector<Vector2f>& textureCoords)
	{
		Vertex& v0 = vertices[v0i];
		Vertex& v1 = vertices[v1i];
		Vertex& v2 = vertices[v2i];

		Vector3f e1 = v1.getPosition() - v0.getPosition();
		Vector3f e2 = v2.getPosition() - v0.getPosition();

		Vector2f uv0 = textureCoords.at(v0.getTextureIndex());
		Vector2f uv1 = textureCoords.at(v1.getTextureIndex());
		Vector2f uv2 = textureCoords.at(v2.getTextureIndex());

		Vector2f deltaUV1 = uv1 - uv0;
		Vector2f deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		Vector3f tangent = ((e1 * deltaUV2.y - e2 * deltaUV1.y) * r).normalise();

		v0.addTangent(tangent);//v0.tangent = tangent;
		v1.addTangent(tangent);//v1.tangent = tangent;
		v2.addTangent(tangent);//v2.tangent = tangent;
	}

	void OBJLoader::SetVertexData(std::vector<Vertex>& vertices, const std::vector<Vector2f>& textureCoords, const std::vector<Vector3f>& normals)
	{
		for (Vertex& vertex : vertices)
		{
			vertex.setTexture(vertex.getTextureIndex() == -1 ? Vector2f() : textureCoords[vertex.getTextureIndex()]);
			vertex.setNormal(vertex.getNormalIndex() == -1 ? Vector3f() : normals[vertex.getNormalIndex()]);
		}
	}

	void OBJLoader::ConvertDataToArrays(const std::vector<Vertex>& vertices, const std::vector<Vector2f>& textures, const std::vector<Vector3f>& normals,
		float* verticesArray, float* texturesArray, float* normalsArray)
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			const Vertex& currentVertex = vertices[i];

			Vector3f position = currentVertex.getPosition();
			Vector2f textureCoord = textures.size() > 0 ? textures[currentVertex.getTextureIndex()] : Vector2f();
			Vector3f normalVector = normals.size() > 0 ? normals[currentVertex.getNormalIndex()] : Vector3f();

			verticesArray[i * 3] = position.x;
			verticesArray[i * 3 + 1] = position.y;
			verticesArray[i * 3 + 2] = position.z;

			texturesArray[i * 2] = textureCoord.x;
			texturesArray[i * 2 + 1] = textureCoord.y;

			normalsArray[i * 3] = normalVector.x;
			normalsArray[i * 3 + 1] = normalVector.y;
			normalsArray[i * 3 + 2] = normalVector.z;
		}
	}

	void OBJLoader::ConvertIndicesToArray(const std::vector<uint16_t> indices, uint16_t* indicesArray)
	{
		for (int i = 0; i < indices.size(); i++)
		{
			indicesArray[i] = indices[i];
		}
	}

	size_t OBJLoader::DealWithAlreadyProcessVertex(Vertex* previousVertex, const int& newTextureIndex, const int& newNormalIndex,
		std::vector<uint16_t>& indices, std::vector<Vertex>& vertices)
	{
		if (previousVertex->hasSameTextureAndNormal(newTextureIndex, newNormalIndex))
		{
			indices.push_back((uint16_t) previousVertex->getIndex());
			return previousVertex->getIndex();
		}
		else
		{
			if (previousVertex->getDuplicateVertexIndex() != -1)
			{
				return DealWithAlreadyProcessVertex(&vertices[previousVertex->getDuplicateVertexIndex()], newTextureIndex, newNormalIndex, indices, vertices);
			}
			else
			{
				Vertex duplicateVertex((uint16_t) vertices.size(), previousVertex->getPosition());
				duplicateVertex.tang_sum = previousVertex->tang_sum;

				duplicateVertex.setTextureIndex(newTextureIndex);
				duplicateVertex.setNormalIndex(newNormalIndex);

				vertices.push_back(duplicateVertex);
				previousVertex->setDuplicateVertexIndex(duplicateVertex.getIndex());
				indices.push_back(duplicateVertex.getIndex());
				return duplicateVertex.getIndex();
			}
		}
	}
};