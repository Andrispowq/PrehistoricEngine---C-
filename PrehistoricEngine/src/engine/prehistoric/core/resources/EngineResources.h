#ifndef PREHISTORIC_ENGINE_RESOURCES_H
#define PREHISTORIC_ENGINE_RESOURCES_H

#include <string>

namespace Prehistoric {
    // Resolve read-only engine resources independently of application working directory.
    // PREHISTORIC_ENGINE_RESOURCES, when set, overrides executable-relative discovery.
    std::string EngineResourcePath(const std::string &relativePath);

    // Stable serialized IDs; only the explicitly migrated legacy paths are aliases.
    std::string CanonicalAssetReference(const std::string &reference);

    // Separate read-only sample package; override with PREHISTORIC_SAMPLE_RESOURCES.
    std::string SampleResourcePath(const std::string &relativePath);

    bool IsSharedAsset(const std::string &reference);

    bool IsEngineAsset(const std::string &reference);

    std::string ResolveAssetPath(const std::string &reference);

    std::string JoinAssetReference(const std::string &directory, const std::string &reference);
}

#endif