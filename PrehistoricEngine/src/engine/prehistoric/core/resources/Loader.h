#ifndef LOADER_H
#define LOADER_H

#include "prehistoric/common/framework/Window.h"

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

		std::optional<void*> LoadResource(bool immidiate, const std::string& path, Extra* extra);
		void ForceLoadQueue();

		void* GetLoadedPointers(size_t& count) const;
	protected:
		Window* window;

		void addToQueue(const std::string& path, Extra* extra);

		std::vector<void*> pointers;
		std::vector<std::pair<std::string, Extra*>> loadQueue;
	};
};

#endif