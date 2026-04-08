#ifndef __JOYSTICK_DATA_H__
#define __JOYSTICK_DATA_H__

#include <Arduino.h>

struct JoystickData {
    uint8_t x = 128; // 0 to 255
    uint8_t y = 128; // 0 to 255
};

#endif