#include "EngineResources.h"
#include "SharedSampleAssets.h"
#include <unordered_set>

#include <cstdlib>
#include <algorithm>
#include <unordered_map>
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

        std::filesystem::path ResourceRoot(const char *package, const char *environment) {
            if (const char *overrideRoot = std::getenv(environment)) {
                if (*overrideRoot)
                    return std::filesystem::path(overrideRoot);
            }
            const auto directory = ExecutableDirectory();
            const auto adjacent = directory / package;
            if (std::filesystem::is_directory(adjacent))
                return adjacent;
            // Premake places each executable in its own application subdirectory.
            const auto shared = directory.parent_path() / package;
            if (std::filesystem::is_directory(shared))
                return shared;
            throw std::runtime_error(
                std::string(package) + " missing beside executable. Build EngineResources or set " + environment + ".");
        }
    }

    std::string EngineResourcePath(const std::string &relativePath) {
        const auto file = ResourceRoot("engine-resources", "PREHISTORIC_ENGINE_RESOURCES") / relativePath;
        if (!std::filesystem::is_regular_file(file))
            throw std::runtime_error("Engine resource missing: " + file.string());
        return file.generic_string();
    }

    std::string SampleResourcePath(const std::string &relativePath) {
        const auto file = ResourceRoot("sample-resources", "PREHISTORIC_SAMPLE_RESOURCES") / relativePath;
        if (!std::filesystem::is_regular_file(file))
            throw std::runtime_error("Sample resource missing: " + file.string());
        return file.generic_string();
    }

    std::string CanonicalAssetReference(const std::string &reference) {
        if (reference.rfind("engine://", 0) == 0 || reference.rfind("sample://", 0) == 0)
            return reference;
        std::string legacy = reference;
        std::replace(legacy.begin(), legacy.end(), '\\', '/');
        while (legacy.rfind("./", 0) == 0)
            legacy.erase(0, 2);
        static const std::unordered_map<std::string, std::string> defaults = {
            {"res/models/cube.obj", "engine://models/cube.obj"},
            {"res/models/prehistoric/cube.pmf", "engine://models/prehistoric/cube.pmf"},
            {"res/models/prehistoric/quad.pmf", "engine://models/prehistoric/quad.pmf"},
            {"res/models/prehistoric/sphere.pmf", "engine://models/prehistoric/sphere.pmf"},
            {"res/models/quad.obj", "engine://models/quad.obj"},
            {"res/models/sphere.obj", "engine://models/sphere.obj"},
            {"res/textures/default.png", "engine://textures/default.png"},
            {"res/textures/logo.png", "engine://textures/logo.png"},
            {"res/textures/noise/Noise256_0.jpg", "engine://textures/noise/noise256_0.jpg"},
            {"res/textures/noise/Noise256_1.jpg", "engine://textures/noise/noise256_1.jpg"},
            {"res/textures/noise/Noise256_2.jpg", "engine://textures/noise/noise256_2.jpg"},
            {"res/textures/noise/Noise256_3.jpg", "engine://textures/noise/noise256_3.jpg"},
            {"res/textures/noise/Noise512_0.jpg", "engine://textures/noise/noise512_0.jpg"},
            {"res/textures/noise/Noise512_1.jpg", "engine://textures/noise/noise512_1.jpg"},
            {"res/textures/noise/Noise512_2.jpg", "engine://textures/noise/noise512_2.jpg"},
            {"res/textures/noise/Noise512_3.jpg", "engine://textures/noise/noise512_3.jpg"},
            {"res/textures/noise/noise256_0.jpg", "engine://textures/noise/noise256_0.jpg"},
            {"res/textures/noise/noise256_1.jpg", "engine://textures/noise/noise256_1.jpg"},
            {"res/textures/noise/noise256_2.jpg", "engine://textures/noise/noise256_2.jpg"},
            {"res/textures/noise/noise256_3.jpg", "engine://textures/noise/noise256_3.jpg"},
            {"res/textures/noise/noise512_0.jpg", "engine://textures/noise/noise512_0.jpg"},
            {"res/textures/noise/noise512_1.jpg", "engine://textures/noise/noise512_1.jpg"},
            {"res/textures/noise/noise512_2.jpg", "engine://textures/noise/noise512_2.jpg"},
            {"res/textures/noise/noise512_3.jpg", "engine://textures/noise/noise512_3.jpg"},
            {"res/textures/prehistoric/default.prt", "engine://textures/prehistoric/default.prt"},
            {"res/textures/prehistoric/dudv1.prt", "engine://textures/prehistoric/dudv1.prt"},
            {"res/textures/prehistoric/logo.prt", "engine://textures/prehistoric/logo.prt"},
            {"res/textures/prehistoric/noise256_0.prt", "engine://textures/prehistoric/noise256_0.prt"},
            {"res/textures/prehistoric/noise256_1.prt", "engine://textures/prehistoric/noise256_1.prt"},
            {"res/textures/prehistoric/noise256_2.prt", "engine://textures/prehistoric/noise256_2.prt"},
            {"res/textures/prehistoric/noise256_3.prt", "engine://textures/prehistoric/noise256_3.prt"},
            {"res/textures/water/dudv1.jpg", "engine://textures/water/dudv1.jpg"},
        };
        const auto found = defaults.find(legacy);
        if (found != defaults.end())
            return found->second;
        static const std::unordered_set<std::string> samples(SharedSampleAssets.begin(), SharedSampleAssets.end());
        if (legacy.rfind("res/", 0) == 0 && samples.count(legacy.substr(4)))
            return "sample://" + legacy.substr(4);
        return reference;
    }

    bool IsEngineAsset(const std::string &reference) {
        return CanonicalAssetReference(reference).rfind("engine://", 0) == 0;
    }

    bool IsSharedAsset(const std::string &reference) {
        const auto canonical = CanonicalAssetReference(reference);
        return canonical.rfind("engine://", 0) == 0 || canonical.rfind("sample://", 0) == 0;
    }

    std::string ResolveAssetPath(const std::string &reference) {
        const auto canonical = CanonicalAssetReference(reference);
        if (canonical.rfind("engine://", 0) == 0)
            return EngineResourcePath(canonical.substr(9));
        if (canonical.rfind("sample://", 0) == 0)
            return SampleResourcePath(canonical.substr(9));
        return reference;
    }

    std::string JoinAssetReference(const std::string &directory, const std::string &reference) {
        // Serialized asset IDs are complete references, not relative filenames.
        if (reference.rfind("engine://", 0) == 0 || reference.rfind("sample://", 0) == 0 || reference.rfind("res/", 0)
            == 0
            || std::filesystem::path(reference).is_absolute())
            return CanonicalAssetReference(reference);
        return CanonicalAssetReference(directory + reference);
    }
}