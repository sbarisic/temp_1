#include <core2.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DISP_MEM_LEN ((SCREEN_WIDTH * SCREEN_HEIGHT) / 8)
#define DISP_MEM_STRIDE (sizeof(disp_mem) / SCREEN_HEIGHT)

#define FONT_W 5
#define FONT_H 8

static const uint8_t digital_font5x7[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, // char ' ' (0x20/32)
    0x00, 0x00, 0x2E, 0x00, 0x00, // char '!' (0x21/33)
    0x06, 0x00, 0x06, 0x00, 0x00, // char '"' (0x22/34)
    0x14, 0x7F, 0x14, 0x7F, 0x14, // char '#' (0x23/35)
    0x06, 0x49, 0x7F, 0x49, 0x30, // char '$' (0x24/36)
    0x24, 0x10, 0x08, 0x24, 0x00, // char '%' (0x25/37)
    0x36, 0x49, 0x36, 0x50, 0x00, // char '&' (0x26/38)
    0x00, 0x00, 0x00, 0x06, 0x00, // char ''' (0x27/39)
    0x00, 0x36, 0x41, 0x00, 0x00, // char ')' (0x29/40)
    0x00, 0x41, 0x36, 0x00, 0x00, // char '(' (0x28/41)
    0x00, 0x08, 0x00, 0x00, 0x00, // char '*' (0x2A/42)
    0x00, 0x08, 0x1C, 0x08, 0x00, // char '+' (0x2B/43)
    0x40, 0x20, 0x00, 0x00, 0x00, // char ',' (0x2C/44)
    0x00, 0x08, 0x08, 0x00, 0x00, // char '-' (0x2D/45)
    0x20, 0x00, 0x00, 0x00, 0x00, // char '.' (0x2E/46)
    0x00, 0x30, 0x06, 0x00, 0x00, // char '/' (0x2F/47)
    0x36, 0x41, 0x41, 0x36, 0x00, // char '0' (0x30/48)
    0x00, 0x00, 0x00, 0x36, 0x00, // char '1' (0x31/49)
    0x30, 0x49, 0x49, 0x06, 0x00, // char '2' (0x32/50)
    0x00, 0x49, 0x49, 0x36, 0x00, // char '3' (0x33/51)
    0x06, 0x08, 0x08, 0x36, 0x00, // char '4' (0x34/52)
    0x06, 0x49, 0x49, 0x30, 0x00, // char '5' (0x35/53)
    0x36, 0x49, 0x49, 0x30, 0x00, // char '6' (0x36/54)
    0x00, 0x01, 0x01, 0x36, 0x00, // char '7' (0x37/55)
    0x36, 0x49, 0x49, 0x36, 0x00, // char '8' (0x38/56)
    0x06, 0x49, 0x49, 0x36, 0x00, // char '9' (0x39/57)
    0x00, 0x14, 0x00, 0x00, 0x00, // char ':' (0x3A/58)
    0x20, 0x14, 0x00, 0x00, 0x00, // char ';' (0x3B/59)
    0x00, 0x08, 0x14, 0x22, 0x00, // char '<' (0x3C/60)
    0x00, 0x14, 0x14, 0x14, 0x00, // char '=' (0x3D/61)
    0x00, 0x22, 0x14, 0x08, 0x00, // char '>' (0x3E/62)
    0x00, 0x01, 0x31, 0x06, 0x00, // char '?' (0x3F/63)
    0x36, 0x49, 0x55, 0x59, 0x2E, // char '@' (0x40/64)
    0x36, 0x09, 0x09, 0x36, 0x00, // char 'A' (0x41/65)
    0x77, 0x49, 0x49, 0x36, 0x00, // char 'B' (0x42/66)
    0x36, 0x41, 0x41, 0x00, 0x00, // char 'C' (0x43/67)
    0x77, 0x41, 0x41, 0x36, 0x00, // char 'D' (0x44/68)
    0x36, 0x49, 0x49, 0x00, 0x00, // char 'E' (0x45/69)
    0x36, 0x09, 0x09, 0x00, 0x00, // char 'F' (0x46/70)
    0x36, 0x41, 0x51, 0x30, 0x00, // char 'G' (0x47/71)
    0x36, 0x08, 0x08, 0x36, 0x00, // char 'H' (0x48/72)
    0x00, 0x00, 0x36, 0x00, 0x00, // char 'I' (0x49/73)
    0x00, 0x40, 0x40, 0x36, 0x00, // char 'J' (0x4A/74)
    0x36, 0x08, 0x14, 0x22, 0x00, // char 'K' (0x4B/75)
    0x36, 0x40, 0x40, 0x00, 0x00, // char 'L' (0x4C/76)
    0x36, 0x01, 0x06, 0x01, 0x36, // char 'M' (0x4D/77)
    0x36, 0x04, 0x10, 0x36, 0x00, // char 'N' (0x4E/78)
    0x36, 0x41, 0x41, 0x36, 0x00, // char 'O' (0x4F/79)
    0x36, 0x09, 0x09, 0x06, 0x00, // char 'P' (0x50/80)
    0x36, 0x41, 0x21, 0x56, 0x00, // char 'Q' (0x51/81)
    0x36, 0x09, 0x19, 0x26, 0x00, // char 'R' (0x52/82)
    0x06, 0x49, 0x49, 0x30, 0x00, // char 'S' (0x53/83)
    0x00, 0x01, 0x37, 0x01, 0x00, // char 'T' (0x54/84)
    0x36, 0x40, 0x40, 0x36, 0x00, // char 'U' (0x55/85)
    0x36, 0x40, 0x36, 0x00, 0x00, // char 'V' (0x56/86)
    0x36, 0x40, 0x30, 0x40, 0x36, // char 'W' (0x57/87)
    0x36, 0x08, 0x08, 0x36, 0x00, // char 'X' (0x58/88)
    0x06, 0x48, 0x48, 0x36, 0x00, // char 'Y' (0x59/89)
    0x20, 0x51, 0x49, 0x45, 0x02, // char 'Z' (0x5A/90)
    0x77, 0x41, 0x41, 0x00, 0x00, // char '[' (0x5B/91)
    0x00, 0x06, 0x30, 0x00, 0x00, // char '\' (0x5C/92)
    0x00, 0x41, 0x41, 0x77, 0x00, // char ']' (0x5D/93)
    0x00, 0x02, 0x01, 0x02, 0x00, // char '^' (0x5E/94)
    0x00, 0x40, 0x40, 0x00, 0x00, // char '_' (0x5F/95)
    0x00, 0x01, 0x02, 0x00, 0x00, // char '`' (0x60/96)
    0x20, 0x54, 0x54, 0x38, 0x40, // char 'a' (0x61/97)
    0x00, 0x36, 0x48, 0x30, 0x00, // char 'b' (0x62/98)
    0x30, 0x48, 0x48, 0x00, 0x00, // char 'c' (0x63/99)
    0x30, 0x48, 0x48, 0x76, 0x00, // char 'd' (0x64/100)
    0x38, 0x54, 0x54, 0x08, 0x00, // char 'e' (0x65/101)
    0x08, 0x6C, 0x0A, 0x00, 0x00, // char 'f' (0x66/102)
    0x08, 0x54, 0x54, 0x38, 0x00, // char 'g' (0x67/103)
    0x36, 0x08, 0x08, 0x30, 0x00, // char 'h' (0x68/104)
    0x00, 0x34, 0x00, 0x00, 0x00, // char 'i' (0x69/105)
    0x00, 0x40, 0x34, 0x00, 0x00, // char 'j' (0x6A/106)
    0x36, 0x10, 0x28, 0x00, 0x00, // char 'k' (0x6B/107)
    0x36, 0x00, 0x00, 0x00, 0x00, // char 'l' (0x6C/108)
    0x30, 0x08, 0x10, 0x08, 0x30, // char 'm' (0x6D/109)
    0x30, 0x08, 0x08, 0x30, 0x00, // char 'n' (0x6E/110)
    0x30, 0x48, 0x48, 0x30, 0x00, // char 'o' (0x6F/111)
    0x78, 0x14, 0x14, 0x08, 0x00, // char 'p' (0x70/112)
    0x08, 0x14, 0x14, 0x68, 0x00, // char 'q' (0x71/113)
    0x30, 0x08, 0x08, 0x00, 0x00, // char 'r' (0x72/114)
    0x08, 0x54, 0x54, 0x20, 0x00, // char 's' (0x73/115)
    0x08, 0x2C, 0x48, 0x00, 0x00, // char 't' (0x74/116)
    0x30, 0x40, 0x40, 0x30, 0x00, // char 'u' (0x75/117)
    0x30, 0x40, 0x30, 0x00, 0x00, // char 'v' (0x76/118)
    0x30, 0x40, 0x20, 0x40, 0x30, // char 'w' (0x77/119)
    0x28, 0x10, 0x10, 0x28, 0x00, // char 'x' (0x78/120)
    0x08, 0x50, 0x50, 0x38, 0x00, // char 'y' (0x79/121)
    0x24, 0x34, 0x2C, 0x24, 0x00, // char 'z' (0x7A/122)
    0x08, 0x36, 0x41, 0x00, 0x00, // char '{' (0x7B/123)
    0x00, 0x36, 0x00, 0x00, 0x00, // char '|' (0x7C/124)
    0x00, 0x41, 0x36, 0x08, 0x00, // char '}' (0x7D/125)
    0x08, 0x08, 0x10, 0x10, 0x00, // char '~' (0x7E/126)
};

uint8_t *disp_mem;
Adafruit_SSD1306 *display;

void c2_disp_convert_xy(int X, int Y, int *Byte, int *Bit)
{
    uint16_t Idx = Y * SCREEN_WIDTH + X;
    *Byte = Idx / 8;
    *Bit = 7 - (Idx % 8);
}

int c2_disp_mem_get_from(int X, int Y, const uint8_t *src)
{
    int Byte = 0;
    int Bit = 0;
    c2_disp_convert_xy(X, Y, &Byte, &Bit);

    if ((src[Byte] >> Bit) & 0x1)
        return 1;

    return 0;
}

void c2_disp_mem_set(int X, int Y, int V)
{
    int Byte = 0;
    int Bit = 0;
    c2_disp_convert_xy(X, Y, &Byte, &Bit);

    uint8_t DispByte = disp_mem[Byte];

    if (V)
        DispByte |= 1 << Bit;
    else
        DispByte &= ~(1 << Bit);

    disp_mem[Byte] = DispByte;
    // printf("MEMSET %2d, %d = %d, %d\n", X, Y, Byte, Bit);
}

void c2_copy_bits(const uint8_t *src, int src_offset_bits, int count, int dst_x, int dst_y)
{
    for (size_t i = 0; i < count; i++)
    {
        int Bit = c2_disp_mem_get_from(src_offset_bits + (count - i), 0, src);

        c2_disp_mem_set(dst_x, dst_y + i, Bit);
    }
}

void c2_blit_char(int dst_x, int dst_y, char chr)
{
    int CharOffset = (chr - 32) * 5;
    int FontW = 5;
    int FontH = 8;

    for (size_t i = 0; i < FontW; i++)
    {
        c2_copy_bits(digital_font5x7, (CharOffset * 8) + FontH * i, FontH, dst_x + i, dst_y);
    }
}

void c2_scroll(int pixels)
{
    int shift_bytes = 16 * pixels;

    for (size_t i = shift_bytes; i < DISP_MEM_LEN; i++)
    {
        disp_mem[i - shift_bytes] = disp_mem[i];
    }

    for (size_t i = 0; i < shift_bytes; i++)
    {
        disp_mem[DISP_MEM_LEN - i - 1] = 0;
    }
}

void c2_printxy(int dst_x, int dst_y, const char *str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        c2_blit_char(dst_x + i * 5, dst_y, str[i]);
    }
}

void c2_clear()
{
    display->clearDisplay();
}

void c2_display(uint16_t color)
{
    display->drawBitmap(0, 0, disp_mem, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    display->display();
}

void core2_oled_print(const char *txt)
{
    dprintf("core2_oled_print: %s\n", txt);

    c2_scroll(FONT_H);
    c2_printxy(0, 7 * FONT_H, txt);

    c2_clear();
    c2_display(1);
}

bool core2_oled_init()
{
    dprintf("core2_oled_init() BEGIN\n");

    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    Wire.begin(5, 4); // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED

    if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false))
    {
        dprintf("display->begin FAILED\n");
        return false;
    }

    disp_mem = (uint8_t *)malloc(DISP_MEM_LEN);
    memset(disp_mem, 0, DISP_MEM_LEN);

    /*for (size_t i = 0; i < 21; i++)
    {
        c2_disp_mem_set(i, 0, 1);
        c2_disp_mem_set(0, i, 1);
        c2_disp_mem_set(i, 20, 1);
        c2_disp_mem_set(20, i, 1);
    }*/

    dprintf("core2_oled_init() END\n");
    return true;
}