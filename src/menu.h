#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <stack>
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

enum MenuEntryType {
    MISC_TYPE,
    MENU_TYPE,
    TEXT_TYPE,
    NUM_TYPE,
    CHECK_TYPE
};

class MenuEntry
{
public:
    MenuEntry();
    virtual ~MenuEntry() {}
    void build(JsonObject &obj);
    static void buildmenu();
    void setType(MenuEntryType t) { type = t; }
    void dump(int level = 0);
    void output(FrameBuffer &s);
    const char *getName() const { return name; }
    const MenuEntryType getType() const { return type; }
    MenuEntry *getSelected()
    {
        if (selected < (int)entries.size())
        {
            return &entries[selected];
        }
        else return NULL;
    }
    void selectplus() { if (++selected >= (int)entries.size()) selected--; }
    void selectminus() { if (selected > 0 ) selected--; }
    void dealLeaf(uint8_t key);
    void dealMenu(uint8_t key);
    // void key(uint8_t k);
    // void render();
    Menu entries;
    void reset();
    String content;
    uint8_t pos;

private:
    MenuEntryType type;
    char name[20];
    int selected;
    int startDisplayAt;
};

typedef std::stack<MenuEntry *> BreadCrumb;
typedef std::map<const char *, Screen> ScreenList;