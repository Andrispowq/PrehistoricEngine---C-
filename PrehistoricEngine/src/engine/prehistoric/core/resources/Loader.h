#ifndef LOADER_H
#define LOADER_H

#include "prehistoric/common/framework/Window.h"

#define NUM_THREADS 16

namespace Prehistoric
{
	struct Extra
	{
	public:
		Extra() {}
		virtual ~Extra() {}
	};

	class Loader
	{
	public:
		Loader(Window* window);
		virtual ~Loader();

		virtual void* LoadResourceInternal(const std::string& path, Extra* extra) = 0;
		virtual void LoadResources() {};

		std::optional<void*> LoadResource(bool immidiate, const std::string& path, Extra* extra, bool extra_heap_allocated = true);
		void ForceLoadQueue();

		/*
			This returns a pointer pointing to the vector of pointers internally stored, so the data should be copied immidietly before it's invalidated
			Flushing the loadQueue again again or deleting the loader automatically invalidates the returned pointer
		*/
		void* GetLoadedPointers(size_t& count) const;
		void FlushPointers();
	protected:
		Window* window;

		void addToQueue(const std::string& path, Extra* extra, bool extra_heap_allocated);

		std::vector<void*> pointers;
		std::vector<std::tuple<std::string, Extra*, bool>> loadQueue;
		std::vector<std::thread> threads;
	};
};

#endif