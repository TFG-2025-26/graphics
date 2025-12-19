#pragma once
#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include "InputState.h"   // Incluye GyroscopeState, KeyboardState, GamepadState
#include "Manager.h"
#include "Singleton.h"
#include <array>

#include "defs.h"

// Forward declarations
using SDL_GameController = struct _SDL_GameController;
using SDL_Sensor = struct _SDL_Sensor;

class InputWrapper;

namespace flux_input {

    class InputManager : public flux_utils::Manager,
        public flux_utils::Singleton<InputManager>
    {
    public:
        FLUX_API friend class flux_utils::Singleton<InputManager>;

        FLUX_API virtual ~InputManager();

        bool init() override;
        void update(float dt) override;
        bool shutdown() override;
        FLUX_API void setVibration(const float& lowVibration, const float& highVibration,
            const float& vibrationTime);
        FLUX_API bool windowShouldClose() const;

        // Getters para acceder a los estados de entrada
        FLUX_API KeyboardState getKeyboardState() const;
        FLUX_API GamepadState getGamepadState() const;
        FLUX_API GyroscopeState getGyroscopeState() const;

    private:
        FLUX_API InputManager();

        InputWrapper* inputWrapper;
        bool quit;

        // --- Teclado ---
        bool keyW = false;
        bool keyA = false;
        bool keyS = false;
        bool keyD = false;
        bool keyUp = false;
        bool keyDown = false;
        bool keyLeft = false;
        bool keyRight = false;
        bool keySpace = false;
        bool keyLShift = false;
        bool keyRShift = false;
        bool keyLControl = false;
        bool keyRControl = false;
        bool keyEnter = false;
        bool keyEscape = false;
        bool keyQ = false;
        bool keyE = false;

        // --- Transformaciones 3D ---
        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;
        float yaw = 0.0f;
        float pitch = 0.0f;

        // --- Gamepad ---
        float leftX = 0.0f;
        float leftY = 0.0f;
        float rightX = 0.0f;
        float rightY = 0.0f;
        bool dpadUp = false;
        bool dpadDown = false;
        bool dpadLeft = false;
        bool dpadRight = false;
        SDL_GameController* gameController = nullptr;
        bool buttonA = false;
        bool buttonB = false;
        bool buttonX = false;
        bool buttonY = false;
        bool buttonBack = false;
        bool buttonGuide = false;
        bool buttonStart = false;
        bool buttonLS = false;
        bool buttonRS = false;
        bool buttonLB = false;
        bool buttonRB = false;
        float buttonLT = 0.0f;
        float buttonRT = 0.0f;

        // --- Sensor (giroscopio) ---
        SDL_Sensor* sensor = nullptr;
        float gyroX = 0.0f;
        float gyroY = 0.0f;
        float gyroZ = 0.0f;

      
        void apply3DTransform();
    };
}

#endif // INPUT_MANAGER_H_
