#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/node/component/light/Light.h"

namespace Prehistoric {
    class Atmosphere : public GameObject {
    public:
        void PreRender(Renderer *renderer) override;

        Vector3f getSunPosition() const { return sun->getParent()->getWorldTransform().getPosition(); }
        inline Light *getSun() const { return sun; }

        void setSun(Light *sun);

        static std::unique_ptr<GameObject> CreateSun();

    private:
        Light *sun = nullptr;
    };
};

#endif