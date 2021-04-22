#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <stack>
#include <LiquidCrystal_I2C.h>

#include "framebuff.h"

class MenuEntry;

typedef std::vector<MenuEntry> Menu;

enum MenuEntryType {
    MISC_TYPE,
    MENU_TYPE,
    TEXT_TYPE,
    NUM_TYPE,
    LEAF_TYPE
};

typedef const char *charset;
struct keyset_t
{
  charset set[4];
  unsigned int nsets;
  bool findKey(unsigned char k, unsigned int& set, unsigned int& pos);
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
    void dealInput(uint8_t key);
    void dealMenu(uint8_t key);
    void deal(uint8_t key);
    Menu entries;
    void reset();
    String content;
    uint8_t pos;
    keyset_t* keyboard;
    unsigned int keyset;
    unsigned int keysetpos;
    void setLeafCallback(void(*cb)(const char*)) { leafCallback = cb; }

private:
    MenuEntryType type;
    void addChar();
    char name[20];
    int selected;
    int startDisplayAt;
    static void(*leafCallback)(const char*);
};

typedef std::stack<MenuEntry *> BreadCrumb;