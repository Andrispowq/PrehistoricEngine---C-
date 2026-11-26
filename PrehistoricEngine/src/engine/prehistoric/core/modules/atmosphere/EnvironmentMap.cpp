#include "Includes.hpp"
#include "EnvironmentMap.h"
#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/resources/EngineResources.h"
#include <stb_image.h>

namespace Prehistoric {
    EnvironmentMap::EnvironmentMap(const std::string &value) { SetAsset(value); }

    void EnvironmentMap::SetAsset(const std::string &value) {
        const auto canonical = CanonicalAssetReference(value);
        const auto path = ResolveAssetPath(canonical);
        int width, height, channels;
        float *pixels = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        if (!pixels) throw std::runtime_error("Cannot load environment map: " + value);
        stbi_image_free(pixels);
        asset = canonical;
    }

    void EnvironmentMap::PreRender(Renderer *renderer) {
        renderer->RegisterEnvironmentMap(*this);
        GameObject::PreRender(renderer);
    }
}