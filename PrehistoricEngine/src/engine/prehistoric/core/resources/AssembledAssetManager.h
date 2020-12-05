#ifndef ASSEMBLED_ASSET_MANAGER_H
#define ASSEMBLED_ASSET_MANAGER_H

#include "engine/prehistoric/common/framework/Window.h"

#include "AssetManager.h"

namespace Prehistoric
{
	class AssembledAssetManager
	{
	public:
		AssembledAssetManager(Window* window);
		~AssembledAssetManager() {}

		/*
			You must load the resource externally, and pass ownership to the manager
		*/
		template<typename Resource>
		size_t loadResource(Resource* resource)
		{
			throw std::string("size_t AssembledAssetManager::loadResource(Resource*) failed, recieved bad type: ") + typeid(Resource).name();
			return -1;
		}

		/*
			Use this function if we are sure that the resource exists
		*/
		template<typename Resource>
		void addReference(size_t ID)
		{
			throw std::string("size_t AssembledAssetManager::addReference(size_t) failed, recieved bad type: ") + typeid(Resource).name();
		}

		template<typename Resource>
		void removeReference(size_t ID)
		{
			throw std::string("size_t AssembledAssetManager::removeReference(size_t) failed, recieved bad type: ") + typeid(Resource).name();
		}

		/*
			Returns the resource pointer (non-owner)
		*/
		template<typename Resource>
		Resource* getResourceByID(size_t ID)
		{
			throw std::string("Resource* AssembledAssetManager::getResourceByID(size_t) failed, recieved bad type: ") + typeid(Resource).name();
			return nullptr;
		}

		template<>
		size_t loadResource<Pipeline>(Pipeline* resource)
		{
			for (auto& elem : pipelines)
			{
				if (elem.second.first.get() == resource)
				{
					return elem.first;
				}
			}

			size_t ret = pipeline_ID++;
			pipelines.insert(std::make_pair(ret, std::make_pair(resource, 0)));
			return ret;
		}

		template<>
		size_t loadResource<Material>(Material* resource)
		{
			for (auto& elem : materials)
			{
				if (elem.second.first.get() == resource)
				{
					return elem.first;
				}
			}

			size_t ret = material_ID++;
			materials.insert(std::make_pair(ret, std::make_pair(resource, 0)));
			return ret;
		}

		template<>
		void addReference<Pipeline>(size_t ID)
		{
			pipelines.at(ID).second++;
		}

		template<>
		void addReference<Material>(size_t ID)
		{
			materials.at(ID).second++;
		}

		template<>
		void removeReference<Pipeline>(size_t ID)
		{
			uint32_t& refCount = pipelines.at(ID).second;
			refCount--;

			if (refCount == 0)
			{
				pipelines.erase(ID);
			}
		}

		template<>
		void removeReference<Material>(size_t ID)
		{
			uint32_t& refCount = materials.at(ID).second;
			refCount--;

			if (refCount == 0)
			{
				materials.erase(ID);
			}
		}

		template<>
		Pipeline* getResourceByID<Pipeline>(size_t ID)
		{
			return pipelines.at(ID).first.get();
		}

		template<>
		Material* getResourceByID<Material>(size_t ID)
		{
			return materials.at(ID).first.get();
		}

		/*
			Getting the whole vector of resources stored
		*/
		template<typename Resource>
		std::vector<Resource*> get()
		{
			throw std::string("size_t AssembledAssetManager::get(void) failed, recieved bad type: ") + typeid(Resource).name();
			return {};
		}

		template<>
		std::vector<Pipeline*> get()
		{
			std::vector<Pipeline*> ret;
			ret.reserve(pipelines.size());

			for (auto it = pipelines.begin(); it != pipelines.end(); it++)
			{
				ret.push_back(it->second.first.get());
			}

			return ret;
		}

		template<>
		std::vector<Material*> get()
		{
			std::vector<Material*> ret;
			ret.reserve(materials.size());

			for (auto it = materials.begin(); it != materials.end(); it++)
			{
				ret.push_back(it->second.first.get());
			}

			return ret;
		}

		AssetManager* getAssetManager() { return manager.get(); }

	private:
		Window* window;

		std::unique_ptr<AssetManager> manager;

		//size_t: ID, pointer: data, uint32_t: refcount
		std::unordered_map<size_t, std::pair<std::unique_ptr<Pipeline>, uint32_t>> pipelines;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Material>, uint32_t>> materials;

		//The IDs that we assign the next asset we add
		size_t pipeline_ID = 0;
		size_t material_ID = 0;

		//These values are just for reference, so I'll mark it as TODO
		constexpr static size_t PipelinesSize = 40;
		constexpr static size_t MaterialsSize = 30;
	};
};

#endif