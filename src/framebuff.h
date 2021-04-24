#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class FrameBuffer : public Stream
{
    public:
    FrameBuffer(uint8_t c, uint8_t r, uint8_t lcdaddress);
    virtual ~FrameBuffer();
    void clear(uint8_t buff = 0);
    void writeField(const int col, const int row, const int length, const char *value);
    bool display();
    void onClear(void(*cb)()) { clearCB = cb;}

    void setCursor(uint8_t col, uint8_t row) { curx = col, cury = row; }
    //   void displayMenu(MenuEntry&, LiquidCrystal_I2C &lcd);

    size_t write(uint8_t c) override;
    void setTitle(const char*);
 
    int available() override { return 0; }
    int read() override { return 0; }
    int peek() override { return 0; }
    // void getCursor(uint8_t& c, uint8_t& r) { c = curx; r = cury; }
    void dump();
    void visibleCursorOn(int8_t c, int8_t r);
    void visibleCursorOff();
    void init();
    void createChar(uint8_t c, const char* g) { lcd.createChar(c,g); }

    private:
    uint8_t cols;
    uint8_t rows;
    uint8_t dispcols;
    uint8_t disprows;
    void (*clearCB)();

    // uint8_t offsetcol;
    // uint8_t offsetrow;
    uint8_t curx;
    uint8_t cury;   
    int8_t vcurx;
    int8_t vcury;   

    char **charArray[2];
    LiquidCrystal_I2C lcd;
};