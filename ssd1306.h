#ifndef SSD1306_H__
#define SSD1306_H__

#include <stdbool.h>
#include <driver/i2c_master.h>

#define SSD1306_MEMORYMODE          0x20 
#define SSD1306_COLUMNADDR          0x21 
#define SSD1306_PAGEADDR            0x22 
#define SSD1306_SETCONTRAST         0x81 
#define SSD1306_CHARGEPUMP          0x8D 
#define SSD1306_SEGREMAP            0xA0 
#define SSD1306_DISPLAYALLON_RESUME 0xA4 
#define SSD1306_NORMALDISPLAY       0xA6 
#define SSD1306_INVERTDISPLAY       0xA7 
#define SSD1306_SETMULTIPLEX        0xA8 
#define SSD1306_DISPLAYOFF          0xAE 
#define SSD1306_DISPLAYON           0xAF 
#define SSD1306_COMSCANDEC          0xC8 
#define SSD1306_SETDISPLAYOFFSET    0xD3 
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5 
#define SSD1306_SETPRECHARGE        0xD9 
#define SSD1306_SETCOMPINS          0xDA 
#define SSD1306_SETVCOMDETECT       0xDB 
#define SSD1306_SETSTARTLINE        0x40 
#define SSD1306_DISABLE_SCROLL      0x2E
#define SSD1306_ACTIVATE_SCROLL     0x2F

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

typedef enum {
    SCROLL_LEFT,
    SCROLL_RIGHT,
    SCROLL_DIAG_RIGHT,
    SCROLL_DIAG_LEFT
} ScrollDirection;

typedef enum {
    PIXEL_OFF = 0,
    PIXEL_ON = 1
} PixelState;

typedef enum {
    SPEED_5_FRAMES = 0x00,
    SPEED_64_FRAMES = 0x01,
    SPEED_128_FRAMES = 0x02,
    SPEED_256_FRAMES = 0x03,
    SPEED_3_FRAMES = 0x04,
    SPEED_4_FRAMES = 0x05,
    SPEED_25_FRAMES = 0x06,
    SPEED_2_FRAMES = 0x07
} ScrollSpeed;

typedef struct {
    int x;
    int y;
} stringPos;

typedef struct {
    uint8_t width;
    uint8_t height;
    i2c_master_dev_handle_t device;
    bool ns;
    unsigned char* buffer;

} puroPixel_SSD1306;

void puroPixel_drawCircle(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t r, float a, uint16_t color);
void puroPixel_drawFillCircle(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t r, uint16_t color);
void puroPixel_invert(puroPixel_SSD1306* display);
unsigned char* puroPixel_getBuffer(puroPixel_SSD1306* display);
void puroPixel_setBuffer(puroPixel_SSD1306* display, unsigned char* newBuffer);
void puroPixel_startScroll(puroPixel_SSD1306* display, ScrollDirection direction, uint8_t start, uint8_t end, ScrollSpeed speed);
void puroPixel_stopScroll(puroPixel_SSD1306* display, bool upd);
void puroPixel_setContrast(puroPixel_SSD1306* display, uint16_t con);
void puroPixel_drawBitmap(puroPixel_SSD1306* display, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
stringPos puroPixel_drawString(puroPixel_SSD1306* display, int16_t x, int16_t y, const char* str, uint8_t scale, uint16_t color, bool textBg, bool textWrap);
void puroPixel_drawFillRect(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t w, int16_t color);
void puroPixel_drawRect(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t w, int16_t color);
void puroPixel_drawHorLine(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t w, int16_t color);
void puroPixel_drawVerLine(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t color);
void puroPixel_fillScreen(puroPixel_SSD1306* display, uint16_t color);
void puroPixel_drawPixel(puroPixel_SSD1306* display, int16_t x, int16_t y, uint16_t color);
bool puroPixel_getPixel(puroPixel_SSD1306* display, int16_t x, int16_t y);
void puroPixel_update(puroPixel_SSD1306* display);
void puroPixel_clear(puroPixel_SSD1306* display);
void puroPixel_begin(puroPixel_SSD1306* display);
void puroPixel_init(
    puroPixel_SSD1306* display,
    uint8_t w,
    uint8_t h,
    i2c_master_dev_handle_t device,
    bool ns
);

#endif