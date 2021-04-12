#include "timedevent.h"

std::forward_list<TimedEvent> TimedEvent::events;

void TimedEvent::start()
{
    next = millis() + period;
    events.push_front(*this);
}

bool TimedEvent::cancel()
{
    bool result = true;
    next = 0;
    events.remove(*this);
    return result;
}

bool TimedEvent::operator()()
{
    bool complete = false;
    if (next != 0)
    {
        if (millis() >= next)
        {
            callback(NULL);
            if (oneshot)
            {
                cancel();
            }
            else
            {
                next += period;
                while (next < millis())
                {
                    // Serial.println("Skip");
                    next += period;
                }
            }
        }
    }
    return complete;
}

void TimedEvent::poll()
{
    for (TimedEvent &ev : events)
    {
        if (ev())
        {
            ev.cancel();
        }
    }
}