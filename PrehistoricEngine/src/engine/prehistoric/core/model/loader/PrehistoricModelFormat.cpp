#include "Includes.hpp"
#include "PrehistoricModelFormat.h"

#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric
{
	Model PrehistoricModelFormat::LoadModel(const std::string& path)
	{
		PR_PROFILE("Prehistoric::PrehistoricModelFormat::LoadModel(), path: " + path);
		Model model;

		std::ifstream stream(path, std::ios::ate | std::ios::binary);
		if (!stream.is_open())
			PR_LOG_RUNTIME_ERROR("Texture file %s couldn't be opened!\n", path.c_str());

		size_t size = (size_t)stream.tellg();
		stream.seekg(0);
		uint8_t* arr = new uint8_t[size];
		stream.read((char*)arr, size);

		RawModelData* raw_data = (RawModelData*)arr;
		uint32_t mesh_count = raw_data->mesh_count;
		uint32_t mat_count = raw_data->material_count;
		uint32_t mesh_table = raw_data->mesh_table_offset;
		uint32_t mat_table = raw_data->material_table_offset;
		uint32_t vertex_data = raw_data->vertex_data_offset;
		uint32_t index_data = raw_data->index_data_offset;
		uint32_t str_data = raw_data->string_data_offset;
		uint32_t file_size = raw_data->file_size;

		if (file_size != size)
		{
			PR_LOG_RUNTIME_ERROR("Prehistoric Model Format file incorrect!\n");
		}

		uint8_t* vertex_part_ptr = arr + vertex_data;
		uint8_t* index_part_ptr = arr + index_data;
		uint8_t* str_part_ptr = arr + str_data;

		uint8_t* mesh_table_ptr = arr + mesh_table;
		for (uint32_t i = 0; i < mesh_count; i++)
		{
			Mesh mesh;
			
			MeshData* mesh_data = (MeshData*)(mesh_table_ptr + i * sizeof(MeshData));
			mesh.setMaterialIndex(mesh_data->material_index);

			float* mesh_vertex_ptr = (float*)(vertex_part_ptr + mesh_data->vertex_offset * sizeof(float));
			for (uint32_t i = 0; i < (mesh_data->vertex_count / 11); i++)
			{
				Vector3f position{ mesh_vertex_ptr[0], mesh_vertex_ptr[1], mesh_vertex_ptr[2] };
				Vector2f texture{ mesh_vertex_ptr[3], mesh_vertex_ptr[4] };
				Vector3f normal{ mesh_vertex_ptr[5], mesh_vertex_ptr[6], mesh_vertex_ptr[7] };
				Vector3f tangent{ mesh_vertex_ptr[8], mesh_vertex_ptr[9], mesh_vertex_ptr[10] };

				mesh.AddVertex({ position, texture, normal, tangent });
				mesh_vertex_ptr += 11;
			}

			uint32_t* mesh_index_ptr = (uint32_t*)(index_part_ptr + mesh_data->index_offset * sizeof(uint32_t));
			for (uint32_t i = 0; i < mesh_data->index_count; i++)
			{
                mesh.AddIndex(*mesh_index_ptr);
				mesh_index_ptr++;
			}

			model.AddMesh(std::move(mesh));
		}

		uint8_t* mat_table_ptr = arr + mat_table;
		for (uint32_t i = 0; i < mat_count; i++)
		{
			Material* material = new Material(manager);
			MaterialData* mat_data = (MaterialData*)(mat_table_ptr + i * sizeof(MaterialData));

			char* name_start = (char*)(str_part_ptr + mat_data->name_offset);
			char* name_end = (char*)(name_start + mat_data->name_length);
			std::string name(name_start, name_end);
			material->setName(name);

			if (mat_data->diffuse_length != 0)
			{
				char* diffuse_start = (char*)(str_part_ptr + mat_data->diffuse_offset);
				char* diffuse_end = (char*)(diffuse_start + mat_data->diffuse_length);
				std::string diff_name(diffuse_start, diffuse_end);
				material->addTexture(ALBEDO_MAP, manager->loadTexture(diff_name, Anisotropic, Repeat).value());
			}
			if (mat_data->normal_length != 0)
			{
				char* normal_start = (char*)(str_part_ptr + mat_data->normal_offset);
				char* normal_end = (char*)(normal_start + mat_data->normal_length);
				std::string normal_name(normal_start, normal_end);
				material->addTexture(NORMAL_MAP, manager->loadTexture(normal_name, Anisotropic, Repeat).value());
			}

			if (mat_data->colourR != -1.0)
			{
				material->addVector3f(COLOUR, { mat_data->colourR, mat_data->colourG, mat_data->colourB });
			}

			//default params
			material->addVector4f(MROT, { /*material.metallic*/ 0, /*material.roughness*/ 0.3f, 1, -1 });
			material->addFloat(EMISSION, 0.0f);

			model.AddMaterial(material);
		}

		return model;
	}
	
	bool PrehistoricModelFormat::SavePrehistoricModel(const std::string& path, Model model)
	{
		std::vector<Mesh> meshes = model.getMeshes();
		std::vector<Material*> materials = model.getMaterials();
		std::map<Texture*, std::string> tex_locations = model.getTextureLocations();

		std::vector<float> vertex_data;
		std::vector<uint32_t> index_data;
		std::vector<char> string_data;

		std::vector<MeshData> mesh_datas;
		for (auto mesh : meshes)
		{
			MeshData mesh_data;
			mesh_data.material_index = mesh.getMaterialIndex();
			mesh_data.vertex_offset = vertex_data.size();
			mesh_data.index_offset = index_data.size();

			std::vector<float> vertices = mesh.GetVertexData();
			std::vector<uint32_t> indices = mesh.getIndices();
			mesh_data.vertex_count = vertices.size();
			mesh_data.index_count = indices.size();

			vertex_data.insert(std::end(vertex_data), std::begin(vertices), std::end(vertices));
			index_data.insert(std::end(index_data), std::begin(indices), std::end(indices));
			
			mesh_datas.push_back(mesh_data);
		}

		std::vector<MaterialData> mat_datas;
		for (auto mat : materials)
		{
			Vector3f mat_colour = mat->getVector3f(COLOUR);

			MaterialData mat_data;
			mat_data.colourR = mat_colour.r;
			mat_data.colourG = mat_colour.g;
			mat_data.colourB = mat_colour.b;

			std::string name = mat->getName();
			mat_data.name_offset = string_data.size();
			mat_data.name_length = name.length();
			string_data.insert(std::end(string_data), std::begin(name), std::end(name));

			Texture* albedo = mat->getTexture(ALBEDO_MAP);
			auto albedo_idx = tex_locations.find(albedo);
			if (albedo_idx != tex_locations.end())
			{
				std::string location = albedo_idx->second;
				mat_data.diffuse_offset = string_data.size();
				mat_data.diffuse_length = location.length();
				string_data.insert(std::end(string_data), std::begin(location), std::end(location));
			}
			else
			{
				mat_data.diffuse_offset = 0;
				mat_data.diffuse_length = 0;
			}

			Texture* normal = mat->getTexture(NORMAL_MAP);
			auto normal_idx = tex_locations.find(normal);
			if (normal_idx != tex_locations.end())
			{
				std::string location = normal_idx->second;
				mat_data.normal_offset = string_data.size();
				mat_data.normal_length = location.length();
				string_data.insert(std::end(string_data), std::begin(location), std::end(location));
			}
			else
			{
				mat_data.normal_offset = 0;
				mat_data.normal_length = 0;
			}

			mat_datas.push_back(mat_data);
		}

		uint64_t fullAllocationSize = sizeof(RawModelData);
		fullAllocationSize += mesh_datas.size() * sizeof(MeshData);
		fullAllocationSize += mat_datas.size() * sizeof(MaterialData);
		fullAllocationSize += vertex_data.size() * sizeof(float);
		fullAllocationSize += index_data.size() * sizeof(uint32_t);
		fullAllocationSize += string_data.size() * sizeof(char);

		uint8_t* fullData = new uint8_t[fullAllocationSize];
		uint32_t full_offset = 0;
		memset(fullData, 0, fullAllocationSize);

		RawModelData* raw_data = (RawModelData*)fullData;
		raw_data->mesh_count = mesh_datas.size();
		raw_data->material_count = mat_datas.size();
		full_offset += sizeof(RawModelData);
		raw_data->mesh_table_offset = full_offset;
		full_offset += mesh_datas.size() * sizeof(MeshData);
		raw_data->material_table_offset = full_offset;
		full_offset += mat_datas.size() * sizeof(MaterialData);
		raw_data->vertex_data_offset = full_offset;
		full_offset += vertex_data.size() * sizeof(float);
		raw_data->index_data_offset = full_offset;
		full_offset += index_data.size() * sizeof(uint32_t);
		raw_data->string_data_offset = full_offset;
		raw_data->file_size = fullAllocationSize;

		uint8_t* data_ptr = (uint8_t*)(raw_data + 1);
		memcpy(data_ptr, mesh_datas.data(), mesh_datas.size() * sizeof(MeshData));
		data_ptr += mesh_datas.size() * sizeof(MeshData);
		memcpy(data_ptr, mat_datas.data(), mat_datas.size() * sizeof(MaterialData));
		data_ptr += mat_datas.size() * sizeof(MaterialData);
		memcpy(data_ptr, vertex_data.data(), vertex_data.size() * sizeof(float));
		data_ptr += vertex_data.size() * sizeof(float);
		memcpy(data_ptr, index_data.data(), index_data.size() * sizeof(uint32_t));
		data_ptr += index_data.size() * sizeof(uint32_t);
		memcpy(data_ptr, string_data.data(), string_data.size() * sizeof(char));

		bool res = SavePrehistoricModel(path, raw_data);

		delete[] fullData;
		return res;
	}

	bool PrehistoricModelFormat::SavePrehistoricModel(const std::string& path, RawModelData* data)
	{
		std::vector<std::string> split = Util::Split(path, '/');

		std::ofstream file("res/models/prehistoric/" + split[split.size() - 1], std::ios::binary);

		if (file.is_open())
		{
			file.write((const char*)data, data->file_size);
			file.close();

			return true;
		}

		return false;
	}
}