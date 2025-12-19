#pragma once

#ifndef LIGHT_H_
#define LIGHT_H_

#include "Component.h"

namespace flux_utils {
    class Vector3;
}

namespace flux_script {
    class ComponentArguments;
}

namespace flux_ec {
    enum LightType {
        POINT,
        DIRECTIONAL,
        SPOTLIGHT
    };

    class CLight : public Component
    {
    public:
        FLUX_API CLight() = default;
        FLUX_API virtual ~CLight();

        bool init(flux_script::ComponentArguments* args) override;
        void update(float dt) override;

        flux_utils::Vector3 getDiffuseColor() const;
        void setDiffuseColor(const flux_utils::Vector3& color);

        static ID getID() { return "LIGHT"; }
        uint8_t getType() const override { return LIGHT; }
    private:
        flux_utils::Vector3* _diffuseColor = nullptr;
        LightType _lightType = LightType::DIRECTIONAL;
    };
}

#endif // LIGHT_H_