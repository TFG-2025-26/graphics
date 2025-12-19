#pragma once

#ifndef INPUT_WRAPPER_H_
#define INPUT_WRAPPER_H_
#pragma warning(disable:4091)
typedef union SDL_Event;
#pragma warning(default:4091)

/// <summary>
/// Clase que gestiona eventos de SDL
/// </summary>
class InputWrapper {
public:
    /// <summary>
    /// Constructora por defecto
    /// </summary>
    InputWrapper();

    /// <summary>
    /// Destructora por defecto
    /// </summary>
    ~InputWrapper();

    /// <summary>
    /// Devuelve el puntero de eventos de SDL
    /// </summary>
    SDL_Event* getEvent();

private:
    SDL_Event* event = nullptr; // evento de SDL
};

#endif // INPUT_WRAPPER_H_