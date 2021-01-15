#include "Includes.hpp"
#include "PlatformUtils.h"

#include "platform/Platform.h"

#include "platform/windows/WindowsPlatformUtils.h"

namespace Prehistoric
{
    FileDialogues* FileDialogues::instance = nullptr;

    FileDialogues* FileDialogues::Get()
    {
        if (!instance)
        {
#if defined(PR_WINDOWS_64)
            instance = new WindowsFileDialogues();
#endif
        }

        return instance;
    }
};
