#include "framebuff.h"

FrameBuffer::FrameBuffer(uint8_t c, uint8_t r)
{
    cols = c;
    rows = r;
    charArray = new char *[rows];
    for (int i = 0; i < rows; i++)
    {
        charArray[i] = new char[cols];
    }
    clear();

    dispcols = 20;
    disprows = 4;
    offsetrow = 0;
    offsetcol = 0;
    curx = 0;
    cury = 0;
}

FrameBuffer::~FrameBuffer()
{
    for (int i = 0; i < rows; i++)
    {
        delete[] charArray[i];
    }
    delete[] charArray;
}

void FrameBuffer::clear()
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            charArray[r][c] = ' '; // '0' + (c % 10);
        }
    }
    offsetrow = 0;
    offsetcol = 0;
    curx = 0;
    cury = 0;
}

bool FrameBuffer::display(LiquidCrystal_I2C &lcd)
{
    for (int r = 0; r < disprows; r++)
    {
        if (r < rows)
        {
            lcd.setCursor(0, r);

            for (int c = 0; c < dispcols; c++)
            {
                if (((c + offsetcol) < cols) && ((r + offsetrow) < rows))
                {
                    lcd.print(charArray[r + offsetrow][c + offsetcol]);
                }
            }
        }
    }
    return true;
}

size_t FrameBuffer::write(uint8_t c)
{
    if (c == '\n')
    {
        // Serial.println("NewLine");
        if (cury < rows)
        {
            cury++;
            curx = 0;
        }
    }
    else
    {
        charArray[cury][curx] = c;
        curx++;
        if (curx >= cols)
        {
            write('\n');
        }
    }
    return 1;
}

void FrameBuffer::setTitle(const char* t)
{
    fixedTitle = true;
    setCursor(0,0);
    print(t);
    setCursor(0,1);
}

/*
void FrameBuffer::displayMenu(MenuEntry& m, LiquidCrystal_I2C &lcd)
{
    clear();
    m.output(*this);
    display(lcd);
}
*/