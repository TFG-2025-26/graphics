#include "InputManager.h"
#include "InputWrapper.h"

#include <SDL_events.h>
#include <SDL_gamecontroller.h>
#pragma warning(disable:4091)
#include <SDL.h>
#pragma warning(default:4091)

#include <iostream>
#include <cmath>

//UTILS MODULE
#include "FluxError.h"

// Declaraciones para funciones de sensores sin incluir <SDL_sensor.h>
extern "C" {
    int SDL_NumSensors(void);
    SDL_Sensor* SDL_SensorOpen(int index);
    void SDL_SensorClose(SDL_Sensor* sensor);
}

namespace flux_input {

    static const float DEAD_ZONE = 0.25f; // Umbral para ejes del gamepad

    InputManager::InputManager()
        : inputWrapper(nullptr), gameController(nullptr), sensor(nullptr), quit(false),
        keyW(false), keyA(false), keyS(false), keyD(false),
        keyUp(false), keyDown(false), keyLeft(false), keyRight(false),
        keySpace(false), keyLShift(false), keyRShift(false),
        keyLControl(false), keyRControl(false), keyEnter(false), keyEscape(false),
        keyQ(false), keyE(false),
        posX(0.0f), posY(0.0f), posZ(0.0f), yaw(0.0f), pitch(0.0f),
        leftX(0.0f), leftY(0.0f), rightX(0.0f), rightY(0.0f),
        dpadUp(false), dpadDown(false), dpadLeft(false), dpadRight(false),
        buttonA(false), buttonB(false), buttonX(false), buttonY(false),
        buttonBack(false), buttonGuide(false), buttonStart(false),
        buttonLS(false), buttonRS(false),
        buttonLB(false), buttonRB(false),
        gyroX(0.0f), gyroY(0.0f), gyroZ(0.0f)
    {

    }

    InputManager::~InputManager() {
        // Recursos se liberan en shutdown()
    }

    bool InputManager::windowShouldClose() const {
        return quit;
    }

    bool InputManager::init() {
        inputWrapper = new InputWrapper();

        // Inicializar subsistemas de game controller y sensores
        if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
            //std::cerr << "Error al inicializar el subsistema de game controller: " << SDL_GetError() << std::endl;
            std::string str(SDL_GetError());
            throwFluxError(false, "Error al inicializar el subsistema de game controller: " + str);
        }
        if (SDL_InitSubSystem(SDL_INIT_SENSOR) != 0) {
            //std::cerr << "Error al inicializar el subsistema de sensores: " << SDL_GetError() << std::endl;
            std::string str(SDL_GetError());
            throwFluxError(false, "Error al inicializar el subsistema de sensores:  " + str);
        }

        // Imprimir el número de sensores detectados
        int numSensors = SDL_NumSensors();
        std::cout << "Número de sensores detectados: " << numSensors << std::endl;

        // Abrir el primer sensor disponible
        if (numSensors > 0) {
            sensor = SDL_SensorOpen(0);
            if (sensor) {
                std::cout << "Primer sensor abierto (no se verifica tipo)." << std::endl;
            }
            else {
                //std::cerr << "No se pudo abrir el sensor 0: " << SDL_GetError() << std::endl;
                std::string str(SDL_GetError());
                throwFluxError(false, "No se pudo abrir el sensor 0: " + str);
            }
        }
        else {
            std::cout << "No se detectaron sensores." << std::endl;
        }

        // Inicializar game controller si hay alguno
        if (SDL_NumJoysticks() > 0 && SDL_IsGameController(0)) {
            gameController = SDL_GameControllerOpen(0);
            if (gameController) {
                std::cout << "Mando conectado correctamente." << std::endl;
            }
            else {
                //std::cerr << "No se pudo abrir el mando: " << SDL_GetError() << std::endl;

                std::string str(SDL_GetError());
                throwFluxError(false, "No se pudo abrir el mando: " + str);
            }
        }

        quit = false;
        isInitialized = true;

        return true;
    }

    void InputManager::update(float dt) {
        // Procesar todos los eventos pendientes
        while (SDL_PollEvent(inputWrapper->getEvent())) {
            SDL_Event* event = inputWrapper->getEvent();
            switch (event->type) {

                // Cierre de ventana
            case SDL_QUIT:
                quit = true;
                break;

                // Eventos de game controller
            case SDL_CONTROLLERDEVICEADDED:
                std::cout << "Se ha añadido un mando." << std::endl;
                break;

            case SDL_CONTROLLERBUTTONDOWN: {
                auto button = event->cbutton.button;
                switch (button) {
                case SDL_CONTROLLER_BUTTON_A:            buttonA = true; break;
                case SDL_CONTROLLER_BUTTON_B:            buttonB = true; break;
                case SDL_CONTROLLER_BUTTON_X:            buttonX = true; break;
                case SDL_CONTROLLER_BUTTON_Y:            buttonY = true; break;
                case SDL_CONTROLLER_BUTTON_BACK:         buttonBack = true; break;
                case SDL_CONTROLLER_BUTTON_GUIDE:        buttonGuide = true; break;
                case SDL_CONTROLLER_BUTTON_START:        buttonStart = true; break;
                case SDL_CONTROLLER_BUTTON_LEFTSTICK:    buttonLS = true; break;
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK:   buttonRS = true; break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  buttonLB = true; break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: buttonRB = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:       dpadUp = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     dpadDown = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     dpadLeft = true; break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    dpadRight = true; break;
                default: break;
                }
                break;
            }

            case SDL_CONTROLLERBUTTONUP: {
                auto button = event->cbutton.button;
                switch (button) {
                case SDL_CONTROLLER_BUTTON_A:            buttonA = false; break;
                case SDL_CONTROLLER_BUTTON_B:            buttonB = false; break;
                case SDL_CONTROLLER_BUTTON_X:            buttonX = false; break;
                case SDL_CONTROLLER_BUTTON_Y:            buttonY = false; break;
                case SDL_CONTROLLER_BUTTON_BACK:         buttonBack = false; break;
                case SDL_CONTROLLER_BUTTON_GUIDE:        buttonGuide = false; break;
                case SDL_CONTROLLER_BUTTON_START:        buttonStart = false; break;
                case SDL_CONTROLLER_BUTTON_LEFTSTICK:    buttonLS = false; break;
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK:   buttonRS = false; break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  buttonLB = false; break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: buttonRB = false; break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:       dpadUp = false; break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     dpadDown = false; break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     dpadLeft = false; break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    dpadRight = false; break;
                default: break;
                }
                break;
            }

            case SDL_CONTROLLERAXISMOTION: {
                Sint16 value = event->caxis.value;
                float normalized = static_cast<float>(value) / 32767.0f;
                if (std::fabs(normalized) < DEAD_ZONE)
                    normalized = 0.0f;
                switch (event->caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX:  leftX = normalized; break;
                case SDL_CONTROLLER_AXIS_LEFTY:  leftY = -normalized; break;
                case SDL_CONTROLLER_AXIS_RIGHTX: rightX = normalized; break;
                case SDL_CONTROLLER_AXIS_RIGHTY: rightY = -normalized; break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                {
                    float normalized = static_cast<float>(value) / 32767.0f;
                    if (normalized < 0.0f) normalized = 0.0f;
                    buttonLT = normalized;
                }
                break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                {
                    float normalized = static_cast<float>(value) / 32767.0f;
                    if (normalized < 0.0f) normalized = 0.0f;
                    buttonRT = normalized;
                }
                break;
                default: break;
                }
                break;
            }

                                         // Procesar eventos del sensor (giroscopio)
            case SDL_SENSORUPDATE: {
                if (sensor) { // Solo si se abrió el sensor
                    const float* data = event->sensor.data;
                    gyroX = data[0];
                    gyroY = data[1];
                    gyroZ = data[2];
                    std::cout << "[Giroscopio] X=" << gyroX
                        << ", Y=" << gyroY
                        << ", Z=" << gyroZ << std::endl;
                }
                break;
            }

                                 // Eventos de teclado
            case SDL_KEYDOWN: {
                SDL_Scancode sc = event->key.keysym.scancode;
                switch (sc) {
                case SDL_SCANCODE_ESCAPE: keyEscape = true; quit = true; break;
                case SDL_SCANCODE_W: keyW = true; break;
                case SDL_SCANCODE_A: keyA = true; break;
                case SDL_SCANCODE_S: keyS = true; break;
                case SDL_SCANCODE_D: keyD = true; break;
                case SDL_SCANCODE_UP: keyUp = true; break;
                case SDL_SCANCODE_DOWN: keyDown = true; break;
                case SDL_SCANCODE_LEFT: keyLeft = true; break;
                case SDL_SCANCODE_RIGHT: keyRight = true; break;
                case SDL_SCANCODE_SPACE: keySpace = true; break;
                case SDL_SCANCODE_LSHIFT: keyLShift = true; break;
                case SDL_SCANCODE_RSHIFT: keyRShift = true; break;
                case SDL_SCANCODE_LCTRL: keyLControl = true; break;
                case SDL_SCANCODE_RCTRL: keyRControl = true; break;
                case SDL_SCANCODE_RETURN: keyEnter = true; break;
                case SDL_SCANCODE_Q: keyQ = true; break;
                case SDL_SCANCODE_E: keyE = true; break;
                default: break;
                }
                break;
            }

            case SDL_KEYUP: {
                SDL_Scancode sc = event->key.keysym.scancode;
                switch (sc) {
                case SDL_SCANCODE_ESCAPE: keyEscape = false; break;
                case SDL_SCANCODE_W: keyW = false; break;
                case SDL_SCANCODE_A: keyA = false; break;
                case SDL_SCANCODE_S: keyS = false; break;
                case SDL_SCANCODE_D: keyD = false; break;
                case SDL_SCANCODE_UP: keyUp = false; break;
                case SDL_SCANCODE_DOWN: keyDown = false; break;
                case SDL_SCANCODE_LEFT: keyLeft = false; break;
                case SDL_SCANCODE_RIGHT: keyRight = false; break;
                case SDL_SCANCODE_SPACE: keySpace = false; break;
                case SDL_SCANCODE_LSHIFT: keyLShift = false; break;
                case SDL_SCANCODE_RSHIFT: keyRShift = false; break;
                case SDL_SCANCODE_LCTRL: keyLControl = false; break;
                case SDL_SCANCODE_RCTRL: keyRControl = false; break;
                case SDL_SCANCODE_RETURN: keyEnter = false; break;
                case SDL_SCANCODE_Q: keyQ = false; break;
                case SDL_SCANCODE_E: keyE = false; break;
                default: break;
                }
                break;
            }

            default:
                break;
            }
        }
        // Aplicar cualquier transformación 3D pendiente (por ejemplo, para la cámara)
        apply3DTransform();
    }

    bool InputManager::shutdown() {
        if (isInitialized) {
            delete inputWrapper;
            inputWrapper = nullptr;

            if (gameController) {
                SDL_GameControllerClose(gameController);
                gameController = nullptr;
            }

            if (sensor) {
                SDL_SensorClose(sensor);
                sensor = nullptr;
            }
            isInitialized = false;
            return true;
        }
        else {
            throwFluxError(false, "No se puede cerrar un modulo que no esta inicializado.");
        }
    }

    KeyboardState InputManager::getKeyboardState() const {
        KeyboardState ks;
        ks.keyW = keyW; ks.keyA = keyA; ks.keyS = keyS; ks.keyD = keyD;
        ks.keyUp = keyUp; ks.keyDown = keyDown; ks.keyLeft = keyLeft; ks.keyRight = keyRight;
        ks.keySpace = keySpace;
        ks.keyLShift = keyLShift; ks.keyRShift = keyRShift;
        ks.keyLControl = keyLControl; ks.keyRControl = keyRControl;
        ks.keyEnter = keyEnter; ks.keyEscape = keyEscape;
        ks.keyQ = keyQ; ks.keyE = keyE;
        return ks;
    }

    GamepadState InputManager::getGamepadState() const {
        GamepadState gs;
        gs.leftX = leftX; gs.leftY = leftY;
        gs.rightX = rightX; gs.rightY = rightY;
        gs.dpadUp = dpadUp; gs.dpadDown = dpadDown; gs.dpadLeft = dpadLeft; gs.dpadRight = dpadRight;
        gs.buttonA = buttonA; gs.buttonB = buttonB; gs.buttonX = buttonX; gs.buttonY = buttonY;
        gs.buttonBack = buttonBack; gs.buttonGuide = buttonGuide; gs.buttonStart = buttonStart;
        gs.buttonLS = buttonLS; gs.buttonRS = buttonRS;
        gs.buttonLB = buttonLB; gs.buttonRB = buttonRB;
        gs.buttonLT = buttonLT; gs.buttonRT = buttonRT;
        return gs;
    }

    GyroscopeState InputManager::getGyroscopeState() const {
        GyroscopeState g;
        g.gyroX = gyroX; g.gyroY = gyroY; g.gyroZ = gyroZ;
        return g;
    }

    void InputManager::apply3DTransform() {
        float radYaw = yaw * 3.14159f / 180.0f;
        float radPitch = pitch * 3.14159f / 180.0f;
        float forwardX = std::cos(radPitch) * std::sin(radYaw);
        float forwardY = std::sin(radPitch);
        float forwardZ = std::cos(radPitch) * std::cos(radYaw);
        // Aquí se podría aplicar la transformación a la cámara o a algún objeto en escena.
    }

    void InputManager::setVibration(const float& lowVibration, 
        const float& highVibration, const float& vibrationTime) {
        if (!gameController) {
            std::cout << "No hay mando conectado." << std::endl;
            return;
        }
        if (SDL_GameControllerHasRumble(gameController)) {
            Uint16 rumbleLow = static_cast<Uint16>(lowVibration * 65535.0f);
            Uint16 rumbleHigh = static_cast<Uint16>(highVibration * 65535.0f);
            Uint32 durationMs = static_cast<Uint32>(vibrationTime);
            int result = SDL_GameControllerRumble(gameController, rumbleLow, rumbleHigh, durationMs);
            if (result != 0) {
                std::cerr << "Error al iniciar vibración: " << SDL_GetError() << std::endl;
                std::string str(SDL_GetError());
                // throwFluxError(,  "Error al iniciar vibración: " + str);
            }
        }
        else {
            std::cout << "El mando no soporta vibración (rumble)." << std::endl;
        }
    }
} // namespace flux_input
