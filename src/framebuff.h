#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class FrameBuffer : public Stream
{
    public:
    FrameBuffer(uint8_t c, uint8_t r);
    virtual ~FrameBuffer();
    void clear();
    bool display(LiquidCrystal_I2C& lcd);

    void setCursor(uint8_t col, uint8_t row) { curx = col, cury = row; }
    //   void displayMenu(MenuEntry&, LiquidCrystal_I2C &lcd);

    size_t write(uint8_t c) override;
    void setTitle(const char*);
 
    int available() override { return 0; }
    int read() override { return 0; }
    int peek() override { return 0; }

    private:
    uint8_t cols;
    uint8_t rows;
    uint8_t dispcols;
    uint8_t disprows;

    uint8_t offsetcol;
    uint8_t offsetrow;
    uint8_t curx;
    uint8_t cury;   
    char **charArray;

    bool fixedTitle;
};