#pragma once
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    class OrbitingSun final : public GameObject {
    public:
        OrbitingSun();

        void PreUpdate(CoreEngine *engine) override;

        std::unique_ptr<GameObject> Clone() const override;

        double GetPhase() const;

        void SetPhase(double seconds);

    private:
        double phaseOffset = 0;
    };
}