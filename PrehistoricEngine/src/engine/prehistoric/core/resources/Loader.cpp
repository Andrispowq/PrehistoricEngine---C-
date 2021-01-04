#include "Includes.hpp"
#include "Loader.h"

namespace Prehistoric
{
	Loader::Loader(Window* window)
	{
	}

	Loader::~Loader()
	{
	}

	std::optional<void*> Loader::LoadResource(bool immidiate, const std::string& path, Extra* extra)
	{
		if (immidiate)
		{
			return LoadResourceInternal(path, extra);
		}
		else
		{
			addToQueue(path, extra);
			return std::nullopt;
		}
	}

	void Loader::ForceLoadQueue()
	{
		for (auto& elem : loadQueue)
		{
			void* result = LoadResourceInternal(elem.first, elem.second);
			pointers.push_back(result);
		}
	}

	void* Loader::GetLoadedPointers(size_t& count) const
	{
		count = pointers.size();
		return (void*)pointers.data();
	}

	void Loader::addToQueue(const std::string& path, Extra* extra)
	{
		loadQueue.emplace_back(std::make_pair(path, extra));
	}
};
