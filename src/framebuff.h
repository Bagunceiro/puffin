#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class FrameBuffer : public Stream
{
    public:
    FrameBuffer(uint8_t c, uint8_t r);
    virtual ~FrameBuffer();
    void clear(uint8_t buff = 0);
    void writeField(const int col, const int row, const int length, const char *value);
    bool display(LiquidCrystal_I2C& lcd);
    void onClear(void(*cb)()) { clearCB = cb;}

    void setCursor(uint8_t col, uint8_t row) { curx = col, cury = row; }
    //   void displayMenu(MenuEntry&, LiquidCrystal_I2C &lcd);

    size_t write(uint8_t c) override;
    void setTitle(const char*);
 
    int available() override { return 0; }
    int read() override { return 0; }
    int peek() override { return 0; }
    void dump();

    private:
    uint8_t cols;
    uint8_t rows;
    uint8_t dispcols;
    uint8_t disprows;
    void (*clearCB)();

    uint8_t offsetcol;
    uint8_t offsetrow;
    uint8_t curx;
    uint8_t cury;   
    char **charArray[2];
};