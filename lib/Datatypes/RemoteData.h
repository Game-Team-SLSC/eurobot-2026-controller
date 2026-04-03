#ifndef __REMOTE_DATA_H__
#define __REMOTE_DATA_H__

#include <Arduino.h>
#include <JoystickData.h>

enum class SWITCH_3_POS: uint8_t {UP, DOWN, MIDDLE};

struct RemoteData {
        JoystickData joystickLeft{};
        JoystickData joystickRight{};

        bool buttons[15] = {
            false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
        };
        // for each button true if pressed
        uint8_t slider = 0; // 0 to 255
        uint8_t score = 0;  // 0 to 255
    };

#endif