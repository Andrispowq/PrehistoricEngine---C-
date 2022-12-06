#ifndef PREHISTORIC_MODEL_FORMAT_H
#define PREHISTORIC_MODEL_FORMAT_H

#include "Includes.hpp"
#include "prehistoric/core/model/Model.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	struct MeshData
	{
		int material_index;
		uint32_t vertex_count;
		uint32_t index_count;
		uint32_t vertex_offset;
		uint32_t index_offset;
	};

	struct MaterialData
	{
		uint32_t name_offset;
		uint32_t name_length;
		uint32_t diffuse_offset;
		uint32_t diffuse_length;
		uint32_t normal_offset;
		uint32_t normal_length;
		float colourR;
		float colourG;
		float colourB;
	};

	struct RawModelData
	{
		uint32_t mesh_count;
		uint32_t material_count;
		uint32_t mesh_table_offset;
		uint32_t material_table_offset;
		uint32_t vertex_data_offset; //vertex_data_offset + index_data_offset + string_data -> full data
		uint32_t index_data_offset;
		uint32_t string_data_offset;
		uint32_t file_size;
		char data[];
	};

	class PrehistoricModelFormat
	{
	public:
		PrehistoricModelFormat() {}
		PrehistoricModelFormat(Window* window, AssetManager* manager)
			: window(window), manager(manager) {}
		~PrehistoricModelFormat() {}

		Model LoadModel(const std::string& path);

	public:
		static bool SavePrehistoricModel(const std::string& path, Model model);
		static bool SavePrehistoricModel(const std::string& path, RawModelData* data);
	private:
		Window* window;
		AssetManager* manager;
	};
};

#endif