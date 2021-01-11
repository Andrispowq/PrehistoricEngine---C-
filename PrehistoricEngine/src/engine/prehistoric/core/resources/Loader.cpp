#include "Includes.hpp"
#include "Loader.h"

namespace Prehistoric
{
	Loader::Loader(Window* window)
		: window(window)
	{
	}

	Loader::~Loader()
	{
	}

	std::optional<void*> Loader::LoadResource(bool immidiate, const std::string& path, Extra* extra, bool extra_heap_allocated)
	{
		if (immidiate)
		{
			return LoadResourceInternal(path, extra);
		}
		else
		{
			addToQueue(path, extra, extra_heap_allocated);
			return std::nullopt;
		}
	}

	void Loader::ForceLoadQueue()
	{
		LoadResources();
	}

	void* Loader::GetLoadedPointers(size_t& count) const
	{
		count = pointers.size();
		return (void*)pointers.data();
	}

	void Loader::FlushPointers()
	{
		pointers.clear();
	}

	void Loader::addToQueue(const std::string& path, Extra* extra, bool extra_heap_allocated)
	{
		loadQueue.emplace_back(std::make_tuple(path, extra, extra_heap_allocated));

		//If we reached our maximum threaded loading capability (16 threads * 4 jobs per thread = 64 loads), we flush the results
		if (loadQueue.size() == 64)
		{
			ForceLoadQueue();
		}
	}
};
