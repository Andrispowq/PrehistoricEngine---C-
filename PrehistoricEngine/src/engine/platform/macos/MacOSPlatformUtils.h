#ifndef MACOS_PLATFORM_UTILS_H
#define MACOS_PLATFORM_UTILS_H

#include "Includes.hpp"
#include "prehistoric/common/util/PlatformUtils.h"

namespace Prehistoric
{
	class MacOSFileDialogues : public FileDialogues
	{
	public:
		MacOSFileDialogues() {}
		~MacOSFileDialogues() {}

		virtual std::optional<std::string> OpenFile(const char* filter) override;
		virtual std::optional<std::string> SaveFile(const char* filter) override;
	};
};

#endif
