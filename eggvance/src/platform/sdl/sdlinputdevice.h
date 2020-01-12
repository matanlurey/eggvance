#pragma once

#ifdef _MSC_VER
#include <SDL2/SDL.h>
#else
#include "SDL.h"
#endif

#include "common/input.h"
#include "platform/inputdevice.h"

class SDLInputDevice : public InputDevice
{
public:
    ~SDLInputDevice();

    void init() override;
    void deinit() override;
    int state() override;

    void deviceEvent(const SDL_ControllerDeviceEvent& event);

    static SDL_Scancode convertKey(Key key);
    static SDL_GameControllerButton convertButton(Button button);

private:
    struct Controls
    {
        InputConfig<SDL_Scancode> keyboard;
        InputConfig<SDL_GameControllerButton> controller;
    } controls;

    int keyboardState() const;
    int controllerState() const;

    SDL_GameController* controller;
};
