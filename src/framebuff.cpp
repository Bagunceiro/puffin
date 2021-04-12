#include "framebuff.h"

FrameBuffer::FrameBuffer(uint8_t c, uint8_t r)
{
    cols = c;
    rows = r;
    charArray[0] = new char *[rows];
    charArray[1] = new char *[rows];

    for (int i = 0; i < rows; i++)
    {
        charArray[0][i] = new char[cols];
        charArray[1][i] = new char[cols];
    }
    clear(0);
    clear(1);

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
        delete[] charArray[0][i];
        delete[] charArray[1][i];
    }
    delete[] charArray[0];
    delete[] charArray[1];
}

void FrameBuffer::clear(uint8_t buf)
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            charArray[buf][r][c] = ' '; // '0' + (c % 10);
        }
    }
    offsetrow = 0;
    offsetcol = 0;
    curx = 0;
    cury = 0;
}

/*
 * write out the current buffer where it differs from the alternative buffer
 * (ie that currently on the display) and then switch buffers.
 */
bool FrameBuffer::display(LiquidCrystal_I2C &lcd)
{
    for (int r = 0; r < disprows; r++)
    {
        if (r < rows)
        {
            lcd.setCursor(0, r);
            for (int c = 0; c < dispcols; c++)
            {
                bool wrote = false;
                if (((c + offsetcol) < cols) && ((r + offsetrow) < rows))
                {
                    if (charArray[0][r + offsetrow][c + offsetcol] != charArray[1][r + offsetrow][c + offsetcol])
                    {
                        if (!wrote)
                        {
                            lcd.setCursor(c + offsetcol, r + offsetrow);
                        }
                        lcd.print(charArray[0][r + offsetrow][c + offsetcol]);
                        charArray[1][r + offsetrow][c + offsetcol] = charArray[0][r + offsetrow][c + offsetcol];
                        wrote = true;
                    }
                    else wrote = false;
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
        charArray[0][cury][curx] = c;
        curx++;
        if (curx >= cols)
        {
            write('\n');
        }
    }
    return 1;
}

void FrameBuffer::setTitle(const char *t)
{
    fixedTitle = true;
    setCursor(0, 0);
    print(t);
    setCursor(0, 1);
}

/*
void FrameBuffer::displayMenu(MenuEntry& m, LiquidCrystal_I2C &lcd)
{
    clear();
    m.output(*this);
    display(lcd);
}
*/