#pragma once

#include <Arduino.h>

class AnalogKeyPad
{
public:
    AnalogKeyPad(void(*down)(uint8_t), void(*up)(uint8_t, unsigned long) = NULL, bool(*tick)(uint8_t, unsigned long) = NULL);
    virtual ~AnalogKeyPad();
    void poll();
    const int8_t curkey();
private:
    uint8_t oldkey;
    void (*keyup)(uint8_t, unsigned long);
    void (*keydown)(uint8_t);
    bool (*keytick)(uint8_t, unsigned long);
    unsigned long keypress;
    unsigned long lastTick;
    bool suppress;
};

#define KEY_LEFT 3
#define KEY_UP 1
#define KEY_DOWN 4
#define KEY_RIGHT 2
#define KEY_OK 5
#define KEY_MENU 6
