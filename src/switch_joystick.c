//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

// Built-in soft-reset combo that triggers a GLFW window close event
#define SOFT_RESET_COMBO (HidNpadButton_Plus|HidNpadButton_Minus|HidNpadButton_L|HidNpadButton_R)

// Internal constants for gamepad mapping source types
#define _GLFW_JOYSTICK_AXIS     1
#define _GLFW_JOYSTICK_BUTTON   2
#define _GLFW_JOYSTICK_HATBIT   3

#define TOUCH_WIDTH             1280
#define TOUCH_HEIGHT            720

enum
{
    _SWITCH_AXIS_LEFT_X,
    _SWITCH_AXIS_LEFT_Y,
    _SWITCH_AXIS_RIGHT_X,
    _SWITCH_AXIS_RIGHT_Y,

    _SWITCH_AXIS_COUNT
};

enum
{
    _SWITCH_BUTTON_A,
    _SWITCH_BUTTON_B,
    _SWITCH_BUTTON_X,
    _SWITCH_BUTTON_Y,
    _SWITCH_BUTTON_LSTICK,
    _SWITCH_BUTTON_RSTICK,
    _SWITCH_BUTTON_L,
    _SWITCH_BUTTON_R,
    _SWITCH_BUTTON_ZL,
    _SWITCH_BUTTON_ZR,
    _SWITCH_BUTTON_PLUS,
    _SWITCH_BUTTON_MINUS,

    _SWITCH_BUTTON_COUNT
};

enum
{
    _SWITCH_HAT_DPAD,
    _SWITCH_HAT_LEFT_STICK,
    _SWITCH_HAT_RIGHT_STICK,

    _SWITCH_HAT_COUNT
};

static _GLFWmapping s_switchMapping =
{
    .name = "Nintendo Switch Controller",
    .guid = "",
    .buttons =
    {
        [GLFW_GAMEPAD_BUTTON_A]            = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_A },
        [GLFW_GAMEPAD_BUTTON_B]            = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_B },
        [GLFW_GAMEPAD_BUTTON_X]            = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_X },
        [GLFW_GAMEPAD_BUTTON_Y]            = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_Y },
        [GLFW_GAMEPAD_BUTTON_BACK]         = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_MINUS },
        [GLFW_GAMEPAD_BUTTON_START]        = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_PLUS },
        //[GLFW_GAMEPAD_BUTTON_GUIDE]        = { .type = _GLFW_JOYSTICK_BUTTON, .index = 0 },
        [GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]  = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_L },
        [GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_R },
        [GLFW_GAMEPAD_BUTTON_LEFT_THUMB]   = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_LSTICK },
        [GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]  = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_RSTICK },
        [GLFW_GAMEPAD_BUTTON_DPAD_UP]      = { .type = _GLFW_JOYSTICK_HATBIT, .index = (_SWITCH_HAT_DPAD<<4) | 0x1 },
        [GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]   = { .type = _GLFW_JOYSTICK_HATBIT, .index = (_SWITCH_HAT_DPAD<<4) | 0x2 },
        [GLFW_GAMEPAD_BUTTON_DPAD_DOWN]    = { .type = _GLFW_JOYSTICK_HATBIT, .index = (_SWITCH_HAT_DPAD<<4) | 0x4 },
        [GLFW_GAMEPAD_BUTTON_DPAD_LEFT]    = { .type = _GLFW_JOYSTICK_HATBIT, .index = (_SWITCH_HAT_DPAD<<4) | 0x8 },
    },
    .axes =
    {
        [GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]   = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_ZL },
        [GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]  = { .type = _GLFW_JOYSTICK_BUTTON, .index = _SWITCH_BUTTON_ZR },
        [GLFW_GAMEPAD_AXIS_LEFT_X]         = { .type = _GLFW_JOYSTICK_AXIS,   .index = _SWITCH_AXIS_LEFT_X,  .axisScale = 1, .axisOffset = 0 },
        [GLFW_GAMEPAD_AXIS_LEFT_Y]         = { .type = _GLFW_JOYSTICK_AXIS,   .index = _SWITCH_AXIS_LEFT_Y,  .axisScale = 1, .axisOffset = 0 },
        [GLFW_GAMEPAD_AXIS_RIGHT_X]        = { .type = _GLFW_JOYSTICK_AXIS,   .index = _SWITCH_AXIS_RIGHT_X, .axisScale = 1, .axisOffset = 0 },
        [GLFW_GAMEPAD_AXIS_RIGHT_Y]        = { .type = _GLFW_JOYSTICK_AXIS,   .index = _SWITCH_AXIS_RIGHT_Y, .axisScale = 1, .axisOffset = 0 },
    },
};

PadState pad;

void _glfwInitSwitchJoysticks(void)
{
    _GLFWjoystick* js = _glfwAllocJoystick(s_switchMapping.name, s_switchMapping.guid,
        _SWITCH_AXIS_COUNT, _SWITCH_BUTTON_COUNT, _SWITCH_HAT_COUNT);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    
    js->mapping = &s_switchMapping;
}

void _glfwUpdateSwitchJoysticks(void)
{
    u64 down, held, up;

    // Read input state
    padUpdate(&pad);
    down = padGetButtonsDown(&pad);
    held = padGetButtons(&pad);
    up   = padGetButtonsUp(&pad);

    // Check for soft-reset combo
    if ((held & SOFT_RESET_COMBO) == SOFT_RESET_COMBO)
    {
        _glfwInputWindowCloseRequest(_glfw.nx.cur_window);
        return;
    }

#define MAP_KEY(_libnx_key, _glfw_key, _scancode) \
    do { \
        if (down & (_libnx_key)) _glfwInputKey(_glfw.nx.cur_window, (_glfw_key), (_scancode), GLFW_PRESS, 0); \
        else if (up & (_libnx_key)) _glfwInputKey(_glfw.nx.cur_window, (_glfw_key), (_scancode), GLFW_RELEASE, 0); \
    } while (0)

    // Map common keyboard keys to the controller
    // TODO: Only do this mapping if a keyboard isn't connected
    MAP_KEY(HidNpadButton_Up, GLFW_KEY_UP, HidKeyboardKey_UpArrow);
    MAP_KEY(HidNpadButton_Down, GLFW_KEY_DOWN, HidKeyboardKey_DownArrow);
    MAP_KEY(HidNpadButton_Left, GLFW_KEY_LEFT, HidKeyboardKey_LeftArrow);
    MAP_KEY(HidNpadButton_Right, GLFW_KEY_RIGHT, HidKeyboardKey_RightArrow);
    MAP_KEY(HidNpadButton_A, GLFW_KEY_X, HidKeyboardKey_X);
    MAP_KEY(HidNpadButton_B, GLFW_KEY_Z, HidKeyboardKey_Z);
    MAP_KEY(HidNpadButton_X, GLFW_KEY_S, HidKeyboardKey_S);
    MAP_KEY(HidNpadButton_Y, GLFW_KEY_A, HidKeyboardKey_A);
    MAP_KEY(HidNpadButton_Plus, GLFW_KEY_ENTER, HidKeyboardKey_Return);
    MAP_KEY(HidNpadButton_Minus, GLFW_KEY_ESCAPE, HidKeyboardKey_Escape);

    // Report touch inputs as mouse clicks
    HidTouchScreenState state={0};
    if (hidGetTouchScreenStates(&state, 1)) {
        if (state.count > 0)
        {

            double scaledXPos = (double)state.touches[0].x / TOUCH_WIDTH * _glfw.nx.scr_width;
            double scaledYPos = (double)state.touches[0].y / TOUCH_HEIGHT * _glfw.nx.scr_height;

            _glfwInputCursorPos(_glfw.nx.cur_window, scaledXPos, scaledYPos);

            if (_glfw.nx.cur_window->mouseButtons[GLFW_MOUSE_BUTTON_LEFT] == GLFW_RELEASE)
                _glfwInputMouseClick(_glfw.nx.cur_window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);

        }
        else if (_glfw.nx.cur_window->mouseButtons[GLFW_MOUSE_BUTTON_LEFT] == GLFW_PRESS)
            _glfwInputMouseClick(_glfw.nx.cur_window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformPollJoystick(_GLFWjoystick* js, int mode)
{
    // Detect presence
    if (!padIsConnected(&pad))
        return GLFW_FALSE;

    if (mode & _GLFW_POLL_AXES)
    {
        HidAnalogStickState left, right;
        left = padGetStickPos(&pad, 0);
        right = padGetStickPos(&pad, 1);
        _glfwInputJoystickAxis(js, _SWITCH_AXIS_LEFT_X,   left.x  / 32768.0f);
        _glfwInputJoystickAxis(js, _SWITCH_AXIS_LEFT_Y,  -left.y  / 32768.0f);
        _glfwInputJoystickAxis(js, _SWITCH_AXIS_RIGHT_X,  right.x / 32768.0f);
        _glfwInputJoystickAxis(js, _SWITCH_AXIS_RIGHT_Y, -right.y / 32768.0f);
    }

    if (mode & _GLFW_POLL_BUTTONS)
    {
        int i;
        u64 keys = padGetButtons(&pad);
        keys |= ((keys >> 24) & 0x3) << _SWITCH_BUTTON_L; // Map KEY_SL/SR_LEFT  into HidNpadButton_L/R
        keys |= ((keys >> 26) & 0x3) << _SWITCH_BUTTON_L; // Map KEY_SL/SR_RIGHT into HidNpadButton_L/R
        for (i = 0; i < _SWITCH_BUTTON_COUNT; i ++)
            _glfwInputJoystickButton(js, i, (keys & BIT(i)) ? GLFW_PRESS : GLFW_RELEASE);
        for (i = 0; i < _SWITCH_HAT_COUNT; i ++)
        {
            // Extract hat values. A small ROR operation is necessary
            // in order to convert Switch DRUL into glfw LDRU
            char hatbits = (keys >> (12 + 4*i)) & 0xF;
            hatbits = (hatbits >> 1) | ((hatbits & 1) << 3);
            _glfwInputJoystickHat(js, i, hatbits);
        }
    }

    return GLFW_TRUE;
}

/* Not used on Switch
void _glfwPlatformUpdateGamepadGUID(char* guid)
{
}
*/