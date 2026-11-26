#pragma once

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    // A distant sky: only rotation affects the rendered lighting.
    class EnvironmentMap : public GameObject {
    public:
        explicit EnvironmentMap(const std::string &asset);

        const std::string &GetAsset() const { return asset; }

        void SetAsset(const std::string &value);

        void PreRender(Renderer *renderer) override;

    private:
        std::string asset;
    };
}