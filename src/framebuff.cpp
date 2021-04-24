#include "framebuff.h"

FrameBuffer::FrameBuffer(uint8_t c, uint8_t r, uint8_t lcdaddress) : lcd(lcdaddress, c, r)
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
    // offsetrow = 0;
    // offsetcol = 0;
    curx = 0;
    cury = 0;
    vcurx = -1;
    vcurx = -1;
}

void FrameBuffer::init()
{
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.display();
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
    if (clearCB)
        clearCB();
    // offsetrow = 0;
    // offsetcol = 0;
    curx = 0;
    cury = 0;
    // dump();
}

/*
 * write out the current buffer where it differs from the alternative buffer
 * (ie that currently on the display) and then switch buffers.
 */
bool FrameBuffer::display()
{
    lcd.blink_off();
    for (int r = 0; r < disprows; r++)
    {
        if (r < rows)
        {
            lcd.setCursor(0, r);
            for (int c = 0; c < dispcols; c++)
            {
                bool wrote = false;
                if (((c) < cols) && ((r) < rows))
                {
                    if (charArray[0][r][c] != charArray[1][r][c])
                    {
                        if (!wrote)
                        {
                            lcd.setCursor(c, r);
                        }
                        lcd.print(charArray[0][r][c]);
                        charArray[1][r][c] = charArray[0][r][c];
                        wrote = true;
                    }
                    else
                        wrote = false;
                }
            }
        }
    }
    if (vcurx >= 0)
    {
        lcd.setCursor(vcurx, vcury);
        lcd.blink_on();
    }
    lcd.display();
    return true;
}

void FrameBuffer::visibleCursorOn(int8_t c, int8_t r)
{
    vcurx = c;
    vcury = r;
    lcd.setCursor(c, r);
    lcd.blink_on();
}

void FrameBuffer::visibleCursorOff()
{
    lcd.blink_off();
    vcurx = -1;
    vcury = -1;
}

/*
void FrameBuffer::dump()
{
    for (int b = 0; b < 2; b++)
    {
        serr.printf("Framebuffer %d\n", b);
        for (int r = 0; r < 4; r++)
        {
            serr.print("|");
            for (int c = 0; c < 20; c++)
            {
                serr.print(charArray[b][r + offsetrow][c + offsetcol]);
            }
            serr.println("|");
        }
    }
}
*/

size_t FrameBuffer::write(uint8_t c)
{
    if (c == '\n')
    {
        // serr.println("NewLine");
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
    writeField(0, 0, 20, t);
    setCursor(0, 1);
}

void FrameBuffer::writeField(const int col, const int row, const int length, const char *value)
{
    setCursor(col, row);

    int l = strlen(value);
    for (int i = 0; i < length; i++)
    {
        if (i < l)
        {
            print(value[i]);
        }
        else
            print(' ');
    }
}