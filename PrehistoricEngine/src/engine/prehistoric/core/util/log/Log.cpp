#include "Includes.hpp"
#include "Log.h"

namespace Prehistoric
{
	std::ofstream* Log::file;

	void Log::Init()
	{
		file = new std::ofstream("./res/profiling/engine.log");
		/*if (!file->is_open())
		{
			PR_LOG_RUNTIME_ERROR("Couldn't open the profiling file!\n");
		}*/
	}

	void Log::Shutdown()
	{
		file->close();
		delete file;
	}
};
