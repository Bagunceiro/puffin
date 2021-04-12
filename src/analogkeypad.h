#pragma once

#include <Arduino.h>

class AnalogKeyPad
{
public:
    AnalogKeyPad(void(*down)(uint8_t), void(*up)(uint8_t) = NULL);
    virtual ~AnalogKeyPad();
    void poll();
    const int8_t curkey();
private:
    uint8_t oldkey;
    void (*keyup)(uint8_t);
    void (*keydown)(uint8_t);
};
