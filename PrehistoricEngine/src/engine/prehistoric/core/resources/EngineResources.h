#ifndef PREHISTORIC_ENGINE_RESOURCES_H
#define PREHISTORIC_ENGINE_RESOURCES_H

#include <string>

namespace Prehistoric {
    // Resolve read-only fonts/shaders independently of application working directory.
    // PREHISTORIC_ENGINE_RESOURCES, when set, overrides executable-relative discovery.
    std::string EngineResourcePath(const std::string &relativePath);
}

#endif