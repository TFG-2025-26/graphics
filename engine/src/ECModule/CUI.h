#pragma once

#ifndef UI_H_
#define UI_H_

#include "Component.h"

namespace flux_utils {
    class Vector2;
    class Vector3;
}

namespace flux_ec {
    class CUI : public Component
    {
    private:
        flux_utils::Vector3* _pos = nullptr;
        flux_utils::Vector2* _size = nullptr;
        flux_utils::Vector3* _color = nullptr;

        std::string _material;
        std::string _font;
        std::string _text;

        float _charHeight = .0f;

        std::string _overlayName;
    public:
        FLUX_API CUI() = default;
        FLUX_API virtual ~CUI();

        void awake() override {}
        bool init(flux_script::ComponentArguments* args) override;
        void update(float dt) override;

        static ID getID() { return "UI"; }
        uint8_t getType() const override { return UI; }

        flux_utils::Vector3 getPosition() const;
        flux_utils::Vector2 getSize() const;

        std::string getMaterial() const;
        flux_utils::Vector3 getColor() const;

        std::string getOverlayName() const;
        std::string getFont() const;
        std::string getText() const;
        float getCharHeight() const;

        std::string getSceneID() const;

        FLUX_API void setText(std::string nt);
    };
}

#endif // UI_H_