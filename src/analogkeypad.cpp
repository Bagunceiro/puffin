#include "analogkeypad.h"

const int8_t AnalogKeyPad::curkey()
{
    int8_t key = 0;
    uint16_t value;

    analogRead(A0);
    value = analogRead(A0);

    if (value > 700)
        key = 0;
    else if (value > 550)
        key = 1;
    else if (value > 425)
        key = 2;
    else if (value > 250)
        key = 3;
    else if (value > 150)
        key = 4;
    else if (value > 10)
        key = 5;
    else
        key = 6;

    return key;
}

void AnalogKeyPad::poll()
{
    uint8_t key = curkey();
    if (key != oldkey)
    {
        if (key == 0)
        {
            if (keyup != NULL)
                keyup(oldkey);
        }
        else
        {
            if (oldkey != 0)
            {
                if (keyup != NULL)
                    keyup(oldkey);
            }
            if (keydown != NULL) keydown(key);
        }
        oldkey = key;
    }
}

AnalogKeyPad::AnalogKeyPad(void(*down)(uint8_t), void(*up)(uint8_t))
{
    oldkey = 0;
    keyup = up;
    keydown = down;
}

AnalogKeyPad::~AnalogKeyPad()
{
}