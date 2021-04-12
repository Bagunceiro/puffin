#include <Arduino.h>

#include <forward_list>

typedef void (*EventCallback)(void*);

class TimedEvent
{
public:
    TimedEvent(const char* i, const EventCallback e, const unsigned long per = 0, bool one = false)
    {
        id = (char*) malloc(strlen(i) + 1);
        strcpy(id, i);
        next = 0;
        callback = e;
        period = per;
        oneshot = one;
    }
    virtual ~TimedEvent()
    {
        free(id);
    }
    bool operator==(const TimedEvent& rhs)
    {
        return this == &rhs;
    }
    void start();
    void pause();
    bool cancel();
    bool operator()();
    static void poll();

private:
    char* id;
    unsigned long next;
    unsigned long period;
    bool oneshot;
    EventCallback callback;
    static std::forward_list<TimedEvent> events;
};