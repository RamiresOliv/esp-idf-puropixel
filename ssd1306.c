#include <stdint.h>
#include <string.h>
#include "ssd1306.h"
#include "font.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "splash.h"
#include "math.h"

// private:

int lerp(int valor, int in_min, int in_max, int out_min, int out_max) {
    return (valor - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} // idk why this exists, whatever

void sendCommand(i2c_master_dev_handle_t dev_handle, uint8_t cmd) {
    uint8_t data[2];
    data[0] = 0x00;   // Control byte: Co=0, D/C#=0 (command)
    data[1] = cmd;
    i2c_master_transmit(dev_handle, data, 2, -1);
}

// public:

/*!
@brief creates a new SSD1306 class. Use this for each display that you have. After defining your display, you are able to begin him and also update his display.
@return SSD1306 object.
@note   after defining your display, make sure to begin device before the own class begin function.
@param addr
    adress of your SSD1306, usually is 0x3C.
@param start
    width of the display.
@param stop
    height of the display.
@param device
    device pointer. Defines as: &device
@param ns
    no splash screen, set to true to disable it. :(
*/
void puroPixel_init(
    puroPixel_SSD1306* display,
    uint8_t w,
    uint8_t h,
    i2c_master_dev_handle_t device,
    bool ns
) {
    display->width = w;
    display->height = h;
    display->device = device;
    display->ns = ns;

    display->buffer = (uint8_t*)malloc(w * (h / 8));
}

/*!
@brief begins the class display. Loads all the required commands and in the end running an clear and a update.
*/
void puroPixel_begin(puroPixel_SSD1306* display) {
    sendCommand(display->device, SSD1306_DISPLAYOFF);
    sendCommand(display->device, SSD1306_SETDISPLAYCLOCKDIV);
    sendCommand(display->device, 0x80);
    sendCommand(display->device, SSD1306_SETMULTIPLEX);
    sendCommand(display->device, 0x3F); // Multiplex ratio (pode precisar de ajustes dependendo da tela)
    sendCommand(display->device, SSD1306_SETDISPLAYOFFSET);
    sendCommand(display->device, 0x0);
    sendCommand(display->device, SSD1306_SETSTARTLINE);
    sendCommand(display->device, SSD1306_CHARGEPUMP);
    sendCommand(display->device, 0x14); // Para telas OLED de 128x64, pode ser necessário
    sendCommand(display->device, SSD1306_MEMORYMODE);
    sendCommand(display->device, 0x00); // Horizontal addressing mode
    sendCommand(display->device, SSD1306_SEGREMAP | 0x1); // Inverter de mapeamento de segmentos socorro
    sendCommand(display->device, SSD1306_COMSCANDEC);
    sendCommand(display->device, SSD1306_SETCOMPINS);
    sendCommand(display->device, 0x12); // Se o display for 128x64
    sendCommand(display->device, SSD1306_SETCONTRAST);
    sendCommand(display->device, 0xCF); // Contraste máximo
    sendCommand(display->device, SSD1306_SETPRECHARGE);
    sendCommand(display->device, 0xF1);
    sendCommand(display->device, SSD1306_SETVCOMDETECT);
    sendCommand(display->device, 0x40); // VCOMH deselected voltage
    sendCommand(display->device, SSD1306_DISABLE_SCROLL);
    sendCommand(display->device, SSD1306_DISPLAYON);

    if (display->ns != true) {
        puroPixel_clear(display);
        puroPixel_drawBitmap(display, 0, 0, epd_bitmap_splash_puro_pixel, 128, 64, 1);
        puroPixel_update(display);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    puroPixel_clear(display);
    puroPixel_update(display);

    //drawPixel(128 / 2, 64 / 2, 1);
    //drawString(0, 0, "Hello", 4, 1, 0, true);
}

/*
@brief clears the current buffer, requires an update to make effect.
@note   after use, call update(). To applay effects.
*/
void puroPixel_clear(puroPixel_SSD1306* display) {
    memset(display->buffer, 0, display->width * (display->height / 8));
}

/*!
@brief load the current buffer to your display. You call this function after a draw or a clear function. For example: drawPixel(...); update(); // loads buffer
*/
void puroPixel_update(puroPixel_SSD1306* display) {
    sendCommand(display->device, SSD1306_PAGEADDR);
    sendCommand(display->device, 0);
    sendCommand(display->device, 0xFF);
    sendCommand(display->device, SSD1306_COLUMNADDR);
    sendCommand(display->device, 0);
    sendCommand(display->device, 128 - 1);

    for (uint8_t page = 0; page < 8; page++) {

        uint8_t data[129];
        data[0] = 0x40;  // Control byte for data

        memcpy(&data[1], &display->buffer[page * 128], 128);

        i2c_master_transmit(display->device, data, 129, -1);
    }
}

// pixel manipulations

/*!
@brief gets the current pixel (from the buffer)
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@return bool
*/
bool puroPixel_getPixel(puroPixel_SSD1306* display, int16_t x, int16_t y) {
    uint16_t index = x + (y / 8) * display->width;
    return (display->buffer[index] >> (y & 7)) & 1;
}

/*!
@brief draws an single pixel. Usefull if you wish to do details or lines.
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_drawPixel(puroPixel_SSD1306* display, int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= display->width) || (y < 0) || (y >= display->height)) {
        return;
    }

    uint16_t index = x + (y / 8) * display->width;
    if (color == 1) {
        display->buffer[index] |= (1 << (y & 7));
    }
    else {
        display->buffer[index] &= ~(1 << (y & 7));
    }
}

/*!
@brief as you might expect, it fills the entire screen.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_fillScreen(puroPixel_SSD1306* display, uint16_t color) {
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 128; x++) {
            puroPixel_drawPixel(display, x, y, color);
        }
    }
}

/*!
@brief draws an perfect single pixel line in the vertical by given axys and height.
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@param h
    height of the line. Basecally an x + h.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_drawVerLine(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t color) {
    for (int cH = 0; cH < h; cH++) {
        puroPixel_drawPixel(display, x + cH, y, color);
    }
}

/*!
@brief draws an perfect single pixel line in the horizontal by given axys and height.
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@param w
    width of the line. Basecally an y + w.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_drawHorLine(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t w, int16_t color) {
    for (int cW = 0; cW < w; cW++) {
        puroPixel_drawPixel(display, x, y + cW, color);
    }
}

/*!
@brief draws an perfect rectangle.
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@param h
    height of the line. Basecally an x + h.
@param w
    width of the line. Basecally an y + w.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_drawRect(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t w, int16_t color) {
    puroPixel_drawVerLine(display, x, y, h, color); // right
    puroPixel_drawHorLine(display, x + h, y, w, color); // down
    puroPixel_drawVerLine(display, x, y + w, h, color); // right (left but starts from right)
    puroPixel_drawHorLine(display, x, y, w, color); // down (up but starts from down)
}

/*!
@brief draws an perfect rectangle but filled.
@param x
    X vector of the pixel, if width is 128, the middle would be 62.
@param y
    Y vector of the pixel, if height is 64, the middle would be 32.
@param h
    height of the line. Basecally an x + h.
@param w
    width of the line. Basecally an y + w.
@param color
    defines the pixel state, 1 = on, 0 = off.
*/
void puroPixel_drawFillRect(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t h, int16_t w, int16_t color) {
    puroPixel_drawVerLine(display, x, y, h, color); // right
    puroPixel_drawHorLine(display, x + h, y, w, color); // down
    puroPixel_drawVerLine(display, x, y + w, h, color); // right (left but starts from right)
    puroPixel_drawHorLine(display, x, y, w, color); // down (up but starts from down)

    for (int cH = 0; cH < h; cH++) {
        for (int cW = 0; cW < w; cW++) {
            puroPixel_drawPixel(display, x + cH, y + cW, color);
        }
    }
}

/*stringPos puroPixel_drawString(int16_t x, int16_t y, const char* str, uint16_t color) {
    int xOffeset = 0;
    int yOffeset = 0;

    bool justChangedLine = false;
    for (int i = 0; i < strlen(str); i++) {
        char character = str[i];

        const char* charF = ASCII[character - 0x20];

        for (int cx = 0; cx < 5; cx++) {
            for (int j = 0; j < 7; j++) {
                uint8_t pixel = (charF[cx] >> j) & 1;

                if (pixel == 1) {
                    // Se color for 1, desenha o pixel com fundo preto
                    if (color == 1) {
                        drawPixel((cx + xOffeset + x), (j + yOffeset + y), pixel);
                    }
                    // Se color for 0, desenha o pixel com fundo branco
                    else {
                        drawPixel((cx + xOffeset + x), (j + yOffeset + y), !pixel);
                    }
                }
            }
        }
        if ((xOffeset + 8) >= 128) {
            xOffeset = 0;
            yOffeset += 8;
            justChangedLine = true;
        }
        else {
            if (justChangedLine) {
                if (character != ' ') {
                    xOffeset += 6;
                }
                justChangedLine = false;
            }
            else {
                xOffeset += 6;
            }
        }
    }
    return { xOffeset, yOffeset + 7 }; // y + 7 of the default offeset
}*/


/*!
@brief draws an string. (to do more stuff here)
@param x
    X vector of the string.
@param y
    Y vector of the string, expect an offset of +7 so, if you wanna put it in the center, (64+7)/2: y should be 35.
@param str
    your string. You can costumize the font later.
@param scale
    the scale of the font. 1 = 1x, 2 = 2x, 3 = 3x, etc. Default is 1.
@param color
    defines the pixels state, 1 = on, 0 = off.
@param textBg
    text should have background? true or false (default is false)
@param textWrap
    defines if text breaks line if not fits. true or false (default is true)
@note   the string is not centered, you have to do it manually. The function will return the offset of the string, so you can use it to center it.
*/
stringPos puroPixel_drawString(puroPixel_SSD1306* display, int16_t x, int16_t y, const char* str, uint8_t scale, uint16_t color, bool textBg, bool textWrap) {
    int xOffset = 0;
    int yOffset = 0;
    int screenWidth = 128;
    int charWidth = 6 * scale;
    int charHeight = 8 * scale;

    for (int i = 0; str[i] != '\0'; i++) {
        char character = str[i];

        // Quebra de linha manual
        if (character == '\n') {
            xOffset = 0;
            yOffset += charHeight;
            continue;
        }

        // Ignora caracteres inválidos
        if (character < 0x20 || character > 0x7F) continue;

        // Verifica quebra automática de linha
        if (textWrap && (xOffset + charWidth > screenWidth)) {
            xOffset = 0;
            yOffset += charHeight;
        }

        const char* charF = ASCII[character - 0x20];

        // Desenha fundo com borda, se ativado
        if (textBg) {
            for (int cx = -1; cx <= 5; cx++) {
                for (int j = -1; j <= 7; j++) {
                    for (int dx = 0; dx < scale; dx++) {
                        for (int dy = 0; dy < scale; dy++) {
                            int16_t px = x + xOffset + (cx * scale) + dx;
                            int16_t py = y + yOffset + (j * scale) + dy;
                            puroPixel_drawPixel(display, px, py, !color);
                        }
                    }
                }
            }
        }

        // Desenha o caractere
        for (int cx = 0; cx < 5; cx++) {
            for (int j = 0; j < 7; j++) {
                uint8_t pixel = (charF[cx] >> j) & 1;
                if (pixel) {
                    for (int dx = 0; dx < scale; dx++) {
                        for (int dy = 0; dy < scale; dy++) {
                            int16_t px = x + xOffset + (cx * scale) + dx;
                            int16_t py = y + yOffset + (j * scale) + dy;
                            puroPixel_drawPixel(display, px, py, color);
                        }
                    }
                }
            }
        }

        xOffset += charWidth;
    }

    stringPos result;
    result.x = xOffset;
    result.y = yOffset + 7 * scale;
    return result;
}

/*!
@brief draws an "image" from an bitmap. Kinda complex to use, but not that hard to understand.
@param x
    X vector of the image. You also have to expect an offeset dependind on the bitmap width.
@param y
    Y vector of the image. Same as X, you have to expect an offeset dependind on the bitmap height.
@param bitmap
    your bitmap.
@param w
    your bitmap width.
@param h
    your bitmap height.
@param color
    defines the pixels state, 1 = on, 0 = off.
*/
void puroPixel_drawBitmap(puroPixel_SSD1306* display, int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t b = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                b <<= 1;
            else
                b = bitmap[j * byteWidth + i / 8];
            if (b & 0x80)
                puroPixel_drawPixel(display, x + i, y, color);
        }
    }
}

/*!
@brief draws an simple circle.
@param x
    X vector of the circle. You also have to expect an offeset dependind on the circle size.
@param y
    Y vector of the image. Same as X, you have to expect an offeset dependind on the circle size.
@param r
    the circle radius.
@param a
    agle/steps, do SMALL increments on this number if you wanna an smooth circle. Recommended is 0.1
@param color
    defines the pixels state, 1 = on, 0 = off.
*/
void puroPixel_drawCircle(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t r, float a, uint16_t color) {
    for (float angle = 0; angle < 2 * M_PI; angle += a) {
        int fx = x + r * cos(angle);
        int fy = y + r * sin(angle);
        puroPixel_drawPixel(display, fx, fy, color);
    }
}

void puroPixel_drawFillCircle(puroPixel_SSD1306* display, int16_t x, int16_t y, int16_t r, uint16_t color) {
    for (int16_t dy = -r; dy <= r; dy++) {
        int16_t dx_limit = sqrt(r * r - dy * dy); // Calcula a largura da linha usando a equação do círculo
        for (int16_t dx = -dx_limit; dx <= dx_limit; dx++) {
            puroPixel_drawPixel(display, x + dx, y + dy, color); // Desenha os pixels horizontalmente
        }
    }
}
/*!
@brief this function invert the polarity. If 1 = 0, 0 = 1, white to black, black to white.
@note   Needs update().
*/

void puroPixel_invert(puroPixel_SSD1306* display) {
    for (int i = 0; i < (display->width * display->height / 8); i++) { // Percorre todo o buffer
        display->buffer[i] = ~display->buffer[i]; // Inverte os bits do byte
    }
}

/*!
@brief gets the current buffer
*/

unsigned char* puroPixel_getBuffer(puroPixel_SSD1306* display) {
    return display->buffer;
}

/*!
@brief replace the buffer to the new one. Before you send make the math and check the buffer size. do: width * (height / 8) and then, you should have it.
@note depending on your display size it MUST MATCH! 8 PAGES! DO THE MATH!

@param b the buffer to be sent. Req Size: [width * (height / 8)]
*/

void puroPixel_setBuffer(puroPixel_SSD1306* display, unsigned char* newBuffer) {
    if (newBuffer == NULL) return;
    display->buffer = newBuffer;
}

/*!
@brief scrolling left or right!
@note   this function doesn't needs an update().
@param direction
    which direction to scroll? SCROLL_LEFT, SCROLL_RIGHT, SCROLL_DIAG_RIGHT, SCROLL_DIAG_LEFT
@param start
    which row to start? 0 = top, 7 = bottom (recomended 0)
@param end
    which row to stop? 0 = top, 7 = bottom (recomended 7)
@param speed
    scroll speed. SPEED_256_FRAMES = slowest, SPEED_2_FRAMES = fastest i.g
@note No need to use puroPixel_update().
*/

void puroPixel_startScroll(puroPixel_SSD1306* display, ScrollDirection direction, uint8_t start, uint8_t end, ScrollSpeed speed) {
    puroPixel_stopScroll(display, false); // Sempre parar qualquer scroll ativo

    if (direction == SCROLL_DIAG_LEFT || direction == SCROLL_DIAG_RIGHT) {
        // 1. Configura área de scroll vertical antes de tudo
        sendCommand(display->device, SSD1306_SET_VERTICAL_SCROLL_AREA);
        sendCommand(display->device, 0x00);    // Área fixa no topo
        sendCommand(display->device, display->height);  // Área que vai rolar

        // 2. Escolhe tipo de scroll
        if (direction == SCROLL_DIAG_LEFT) {
            sendCommand(display->device, SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
        }
        else {
            sendCommand(display->device, SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
        }

        // 3. Argumentos do scroll diagonal
        sendCommand(display->device, 0x00);  // Dummy
        sendCommand(display->device, start); // Página inicial
        sendCommand(display->device, speed); // Velocidade
        sendCommand(display->device, end);   // Página final
        sendCommand(display->device, 0x01);  // Vertical offset (mínimo 1 pra se mover)
        sendCommand(display->device, 0xFF);  // Dummy

        // 4. Ativa o scroll
        sendCommand(display->device, SSD1306_ACTIVATE_SCROLL);
    }
    else {
        // Scroll horizontal simples
        if (direction == SCROLL_RIGHT) {
            sendCommand(display->device, SSD1306_RIGHT_HORIZONTAL_SCROLL);
        }
        else {
            sendCommand(display->device, SSD1306_LEFT_HORIZONTAL_SCROLL);
        }

        // Argumentos do scroll horizontal
        sendCommand(display->device, 0x00);  // Dummy
        sendCommand(display->device, start); // Página inicial
        sendCommand(display->device, speed); // Velocidade
        sendCommand(display->device, end);   // Página final
        sendCommand(display->device, 0x00);  // Dummy
        sendCommand(display->device, 0xFF);  // Dummy

        sendCommand(display->device, SSD1306_ACTIVATE_SCROLL);
    }
}

/*!
@brief stops the scroll, usually when stops it not return to the middle. To fix that after the stop command, is executed automatically an puroPixel_update() but you can disable by setting the update param.
@note happening unwanted updates after stopping scroll? try set update param to false.
@param update
    sets at the end an puroPixel_update() to return the display to the center.
*/

void puroPixel_stopScroll(puroPixel_SSD1306* display, bool update) {
    sendCommand(display->device, SSD1306_DISABLE_SCROLL);
    if (update) puroPixel_update(display);
}

/*!
@brief changes the contrast of the OLED display.
@note this command doesn't needs puroPixel_update() and i don't know if highest contrast can damage the leds, but just in case, use 207 as the maximum. And wow, the 0 is really nothing.
@param con
    min: 0, max: 255, recommended: 207 (also u can use hex 0xFF etc...)
*/

void puroPixel_setContrast(puroPixel_SSD1306* display, uint16_t con) {
    sendCommand(display->device, SSD1306_SETCONTRAST);
    sendCommand(display->device, con);
}