#include "Includes.hpp"
#include "PrehistoricDataFile.h"

namespace Prehistoric
{
	PrehistoricDataFile::PrehistoricDataFile(AssembledAssetManager* manager, const std::string& location)
		: manager(manager)
	{
		file.open(location);

		if (!file.is_open())
		{
			PR_LOG_ERROR("Could not open Prehistoric Data File at location %s\n", location);
		}

		if (!Initialise())
		{
			PR_LOG_ERROR("Could not initialise the data file!\n");
		}
	}

	PrehistoricDataFile::~PrehistoricDataFile()
	{
		file.close();
	}

	bool PrehistoricDataFile::Initialise()
	{
		file.seekg(0);
		file.read((char*)&master_header, sizeof(DataFileMasterHeader));
		if (master_header.magic != PRDF_MASTER_MAGIC)
		{
			return false;
		}

		uint32_t id_table_offset = master_header.id_name_table_offset;
		uint32_t id_table_size = master_header.id_name_table_size;

		uint8_t* buffer = new uint8_t[id_table_size];
		file.seekg(id_table_offset);
		file.read((char*)buffer, id_table_size);

		IDNameTableEntry* entry = (IDNameTableEntry*)buffer;
		uint32_t size_traversed = 0;
		for (size_traversed = 0; size_traversed < id_table_size;)
		{
			char buffer[255] = { 0 };
			memcpy(buffer, entry->characters, entry->size - 8);
			std::string name = buffer;
			ID_name_map.emplace(std::make_pair(entry->ID, name));

			size_traversed += entry->size;
		}

		return true;
	}

	Model PrehistoricDataFile::LoadModel(const std::string& path)
	{
		uint32_t lower = hash(path);
		return Model();
	}

	void PrehistoricDataFile::WriteDataFile(AssembledAssetManager* manager, const std::string& location)
	{
		std::ofstream out(location);

		
	}

	uint32_t PrehistoricDataFile::hash(const std::string& str)
	{
		constexpr uint32_t MULTIPLIER = 32;

		uint32_t h = 0;
		for (uint8_t* p = (uint8_t*)str.c_str(); *p != 0; p++)
		{
			h = MULTIPLIER * h + *p;
		}

		return h;
	}
}