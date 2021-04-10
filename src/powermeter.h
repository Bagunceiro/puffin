#pragma once

#include <PZEM004Tv30.h>

class PowerMeter
{
public:
    PowerMeter();
    void operator()();
    PZEM004Tv30 pzem;
};