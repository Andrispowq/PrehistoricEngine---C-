#ifndef PREHISTORIC_DATA_FILE_LOADER_H
#define PREHISTORIC_DATA_FILE_LOADER_H

#include "Includes.hpp"
#include "prehistoric/core/model/Model.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	constexpr static uint32_t PRDF_MASTER_MAGIC = 0x50544446;
	constexpr static uint32_t PRDF_VERSION = 0x01000001;

	enum class DataType : uint16_t
	{
		Mesh,
		Material,
		Model,
		Shader,
		Audio
	};

	struct DataFileMasterHeader
	{
		uint32_t magic;
		uint32_t version;
		uint32_t id_name_table_offset;
		uint32_t id_name_table_size;
		uint32_t header_offset;
		uint32_t header_size;
		uint32_t data_offset;
		uint32_t data_size;
		uint32_t unused[8];
	};

	struct IDNameTableEntry
	{
		uint32_t size;
		uint64_t ID;
		uint8_t characters[];
	};

	struct DataFileHeader //must be packed
	{
		uint64_t ID;
		uint64_t size;
		DataType type;
		uint16_t unused;
	};

	struct ModelData
	{
		uint32_t mesh_count;
		uint32_t material_count;
		void* data;
	};

	class PrehistoricDataFile
	{
	public:
		PrehistoricDataFile(AssembledAssetManager* manager, const std::string& location);
		~PrehistoricDataFile();

		bool Initialise();

		Model LoadModel(const std::string& path);

		void WriteDataFile(AssembledAssetManager* manager, const std::string& location);

	private:
		uint32_t hash(const std::string& str);

	private:
		std::fstream file;
		AssembledAssetManager* manager;

		DataFileMasterHeader master_header;

		std::map<uint64_t, std::string> ID_name_map;
	};
}

#endif