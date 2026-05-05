#pragma once

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "Component.h"

namespace flux_render {
    class RenderObject;
}

namespace flux_utils {
    class Vector3;
    class Vector4;
}

namespace flux_script {
    class ComponentArguments;
}

namespace flux_ec {
    class CTransform : public Component
    {
    public:
        FLUX_API CTransform() = default;
        FLUX_API virtual ~CTransform();

        FLUX_API bool init(flux_script::ComponentArguments* args) override;
        void update(float dt) override;

        FLUX_API flux_utils::Vector3 getPos() const;
        FLUX_API flux_utils::Vector4 getRot() const;
        FLUX_API flux_utils::Vector3 getScale() const;

        FLUX_API void setPos(const flux_utils::Vector3& pos);
        FLUX_API void setRot(const flux_utils::Vector4& rot);
        FLUX_API void setScale(const flux_utils::Vector3& scale);

        FLUX_API void lookAt(const flux_utils::Vector3& pos);

        static ID getID() { return "TRANSFORM"; }
        uint8_t getType() const override { return TRANSFORM; }
    private:
        flux_utils::Vector3* _pos = nullptr;
        flux_utils::Vector4* _rot = nullptr;
        flux_utils::Vector3* _scale = nullptr;

        flux_render::RenderObject* _renderObject = nullptr;

        void syncRenderTransform();
    };
}


#endif // TRANSFORM_H_