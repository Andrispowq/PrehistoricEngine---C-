#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#include "Includes.hpp"

namespace Prehistoric
{
	class FileDialogues
	{
	public:
		static FileDialogues* Get();

		virtual std::optional<std::string> OpenFile(const char* filter) = 0;
		virtual std::optional<std::string> SaveFile(const char* filter) = 0;

	protected:
		FileDialogues() {}
		~FileDialogues() {}
	private:
		static FileDialogues* instance;
	};
};

#endif