#pragma once
#ifndef INPUT_STATE_H_
#define INPUT_STATE_H_

namespace flux_input {

    // Estructura para almacenar el estado del teclado
    struct KeyboardState {
        bool keyW;
        bool keyA;
        bool keyS;
        bool keyD;
        bool keyUp;
        bool keyDown;
        bool keyLeft;
        bool keyRight;
        bool keySpace;
        bool keyLShift;
        bool keyRShift;
        bool keyLControl;
        bool keyRControl;
        bool keyEnter;
        bool keyEscape;
        bool keyQ;
        bool keyE;
    };

    // Estructura para almacenar el estado del gamepad
    struct GamepadState {
        float leftX;
        float leftY;
        float rightX;
        float rightY;
        bool dpadUp;
        bool dpadDown;
        bool dpadLeft;
        bool dpadRight;
        bool buttonA;
        bool buttonB;
        bool buttonX;
        bool buttonY;
        bool buttonBack;
        bool buttonGuide;
        bool buttonStart;
        bool buttonLS;
        bool buttonRS;
        bool buttonLB;
        bool buttonRB;
        float buttonLT;
        float buttonRT;
    };

    // NUEVO: Estructura para almacenar el estado del giroscopio
    struct GyroscopeState {
        float gyroX;
        float gyroY;
        float gyroZ;
    };

} // namespace flux_input

#endif // INPUT_STATE_H_
