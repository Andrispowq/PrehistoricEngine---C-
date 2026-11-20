#include "Includes.hpp"
#include "PlatformUtils.h"

#include "platform/Platform.h"

#if defined(PR_WINDOWS_64)
#include "platform/windows/WindowsPlatformUtils.h"
#elif defined(PR_PLATFORM_MACOS)
#include "platform/macos/MacOSPlatformUtils.h"
#endif

namespace Prehistoric
{
    FileDialogues* FileDialogues::instance = nullptr;

    FileDialogues* FileDialogues::Get()
    {
        if (!instance)
        {
#if defined(PR_WINDOWS_64)
            instance = new WindowsFileDialogues();
#elif defined(PR_PLATFORM_MACOS)
            instance = new MacOSFileDialogues();
#endif
        }

        return instance;
    }
};
