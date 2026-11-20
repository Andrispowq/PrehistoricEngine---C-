#include "EngineResources.h"

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace Prehistoric {
    namespace {
        std::filesystem::path ExecutableDirectory() {
#if defined(_WIN32)
            std::vector<wchar_t> buffer(32768);
            DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (!length || length >= buffer.size())
                throw std::runtime_error("Cannot determine executable path for engine resources");
            return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
#elif defined(__APPLE__)
            uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size);
            std::vector<char> buffer(size);
            if (_NSGetExecutablePath(buffer.data(), &size) != 0)
                throw std::runtime_error("Cannot determine executable path for engine resources");
            return std::filesystem::canonical(buffer.data()).parent_path();
#else
            std::vector<char> buffer(4096);
            auto length = readlink("/proc/self/exe", buffer.data(), buffer.size());
            if (length <= 0 || static_cast<size_t>(length) >= buffer.size())
                throw std::runtime_error("Cannot determine executable path for engine resources");
            return std::filesystem::path(std::string(buffer.data(), length)).parent_path();
#endif
        }

        std::filesystem::path ResourceRoot() {
            if (const char *overrideRoot = std::getenv("PREHISTORIC_ENGINE_RESOURCES")) {
                if (*overrideRoot)
                    return std::filesystem::path(overrideRoot);
            }
            const auto directory = ExecutableDirectory();
            const auto adjacent = directory / "engine-resources";
            if (std::filesystem::is_directory(adjacent))
                return adjacent;
            // Premake places each executable in its own application subdirectory.
            const auto shared = directory.parent_path() / "engine-resources";
            if (std::filesystem::is_directory(shared))
                return shared;
            throw std::runtime_error(
                "Engine resources missing beside executable. Build EngineResources or set PREHISTORIC_ENGINE_RESOURCES.");
        }
    }

    std::string EngineResourcePath(const std::string &relativePath) {
        const auto file = ResourceRoot() / relativePath;
        if (!std::filesystem::is_regular_file(file))
            throw std::runtime_error("Engine resource missing: " + file.string());
        return file.string();
    }
}