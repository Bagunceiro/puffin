#include "analogkeypad.h"

const int8_t AnalogKeyPad::curkey()
{
    int8_t key = 0;
    uint16_t value;

    analogRead(A0);
    value = analogRead(A0);

    if (value > 800)
        key = 0;
    else if (value > 600)
        key = 1;
    else if (value > 480)
        key = 2;
    else if (value > 325)
        key = 3;
    else if (value > 175)
        key = 4;
    else if (value > 45)
        key = 5;
    else
        key = 6;

    return key;
}

void AnalogKeyPad::poll()
{
    uint8_t key = curkey();
    unsigned long now = millis();
    unsigned long sincepress = now - keypress;
    if (key != oldkey)
    {
        if (key == 0)
        {
            if ((keyup != NULL) && (!suppress))
                keyup(oldkey, sincepress);
            suppress = false;
        }
        else
        {
            if (oldkey != 0)
            {
            if ((keyup != NULL) && (!suppress))
                    keyup(oldkey, sincepress);
                suppress = false;
            }
            keypress = now;
            if (keydown != NULL)
                keydown(key);
        }
        oldkey = key;
    }
    else
    {
        if (key != 0)
        {
            // Call keytick every 1/4 second while key is pressed
            if ((now - lastTick) >= 250)
            {
                if ((keytick != NULL) && (!suppress))
                {
                    if (keytick(key, sincepress))
                    {
                        // Serial.println("Suppress the keyup");
                        suppress = true;
                    }
                }
                lastTick = now;
            }
        }
    }
}

AnalogKeyPad::AnalogKeyPad(void (*down)(uint8_t), void (*up)(uint8_t, unsigned long), bool (*tick)(uint8_t, unsigned long))
{
    oldkey = 0;
    keyup = up;
    keydown = down;
    keytick = tick;
}

AnalogKeyPad::~AnalogKeyPad()
{
}