#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <LiquidCrystal_I2C.h>

#include "framebuff.h"

class ScreenField
{
public:
    ScreenField(const uint8_t _x, const uint8_t _y, const uint8_t _l);
    ScreenField();
    virtual ~ScreenField();

    bool overlaps(const ScreenField &field) const;
    uint8_t start() const { return x; }
    uint8_t end() const { return length + x - 1; }

    int display(const char *val);

    void dump() const
    {
        Serial.printf(" %d, %d, %d\n", x, y, length);
    }

private:
    uint8_t x;
    uint8_t y;
    uint8_t length;
};

typedef std::map<const char *, ScreenField> ScreenFieldList;

class Screen
{
public:
    enum idcode
    {
        MAIN_SCREEN,
        TOP_MENU,
        WIFI_MENU
    };
    Screen(const uint8_t _cols = 20, const uint8_t _rows = 4);
    Screen(const Screen &);
    Screen &operator=(const Screen &);
    virtual ~Screen();

    bool addField(const char *name, const ScreenField &f);
    int display(const char *fieldname, const char *val);
    static void build();

    void dump();

private:
    ScreenFieldList fields;
    uint8_t cols;
    uint8_t rows;
};

class MenuEntry;

typedef std::vector<MenuEntry> Menu;

class MenuEntry
{
    public:
    MenuEntry();
    virtual ~MenuEntry() {}
    void build(JsonObject& obj);
    static void buildmenu();
    const char* setName(const char* n);
    const char* setType(const char* n);
    void dump(int level = 0);
    void output(FrameBuffer& s);
    Menu entries;
    private:
    char type[10];
    char name[20];
};

typedef std::map<const char *, Screen> ScreenList;