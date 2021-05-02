#include "avr.h"

// ---------------------------------------------------------------------
// Отображение видеобуфера на экран
// ---------------------------------------------------------------------

// Обновление экрана, в зависимости от текущего режима работы
void APP::update_screen() { if (ds_debugger) ds_update(); else display_update(); }

// Обновить текст в (X, Y)
void APP::update_text_xy(int X, int Y) {

    if (videomode)
        return;

    // PAGE 2: Страница символов и палитры
    // PAGE 3: Страница знакогенератора

    int k;
    int addr = BASE_TEXT + (160*Y + 2*X);
    int ch   = sram[addr + 0];
    int attr = sram[addr + 1];
    int xshift = (width  - 640) / 2,
        yshift = (height - 400) / 2;

    for (int y = 0; y < 16; y++) {

        int ft = sram[BASE_TEXT + 0x1000 + 16*ch + y];
        for (int x = 0; x < 8; x++) {

            int cbit  = ft & (1 << (7 - x));
            int cursor = (cursor_x == X && cursor_y == Y) && (y >= 14) ? 1 : 0;
            int color = cbit ^ (flash & cursor) ? (attr & 0x0F) : (attr >> 4);

            int gb = sram[BASE_TEXT + 0xFA0 + 2*color];
            int  r = sram[BASE_TEXT + 0xFA1 + 2*color];

            // Вычисляется цвет
            color = ((gb & 0x0F) << 4) | ((gb & 0xF0) << 8) | ((r & 0x0F) << 20);

            // Когда экран скомпилирован под мелкий монитор
            if (height <= 480) {
                pset(8*X + x + xshift, 16*Y + y + yshift, color);
            }
            else
                for (int k = 0; k < 4; k++) {
                    pset(2*(8*X + x) + k%2, 2*(16*Y + y) + k/2, color);
                }
        }
    }
}

// 0xF000 - 0xFFFF Видеопамять
void APP::update_byte_scr(int addr) {

    int base, x, y, v, cl;
    if (ds_debugger) return;

    switch (videomode) {

        // 80x25 (8k)
        case 0:

            if ((addr >= BASE_TEXT) && (addr < BASE_TEXT + 0xFA0)) {

                addr = (addr - BASE_TEXT) >> 1;
                update_text_xy(addr % 80, addr / 80);
            }
            // Обновить весь дисплей - меняется цвет в палитре
            else if ((addr >= BASE_TEXT + 0x0FA0 && addr <  BASE_TEXT + 0x0FC0) ||  // Палитра
                     (addr >= BASE_TEXT + 0x1000 && addr <= BASE_TEXT + 0x1FFF))    // Знакоместо
                     { require_disp_update = 1; }

            break;

        // 640x400x4 (128k)
        case 1:

            base = 0xF000 + 4096*0x20;
            if (addr >= base && addr < base + 128*1024) {

                v = sram[addr];
                addr -= base;
                x = addr % 320;
                y = addr / 320;

                for (int i = 0; i < 2; i++) {

                    cl = (i == 0 ? v >> 4 : v) & 15;
                    for (int k = 0; k < 4; k++) pset(4*x + 2*i + (k&1), 2*y + (k>>1), DOS_13[cl]);
                }
            }

            break;

        // 320x200x8 (64k x 2)
        case 2:
        case 3:

            base = 0xF000 + 4096*(videomode == 3 ? 0x30 : 0x20);
            if ((addr >= base) && (addr < base + 65536)) {

                v = sram[addr];
                addr -= base;
                x = addr % 320;
                y = addr / 320;

                // 8bit -> 24bit
                v = ((v&3)<<6) | ((v&0x1C)<<11) | ((v&0xE0)<<16);

                for (int k = 0; k < 16; k++) pset(4*x + (k&3), 4*y + (k>>2), v);
            }

            break;
    }
}

// Обновить весь экран
void APP::display_update() {

    int k, x, y;

    cls(0);

    switch (videomode) {

        // TEXT 80x25
        case 0:

            for (y = 0; y < 25; y++)
            for (x = 0; x < 80; x++)
                update_text_xy(x, y);

            break;

        case 1: for (k = 0; k < 131072; k++) update_byte_scr(0xF000 + 0x20*4096 + k); break;
        case 2: for (k = 0; k < 65536;  k++) update_byte_scr(0xF000 + 0x20*4096 + k); break;
        case 3: for (k = 0; k < 65536;  k++) update_byte_scr(0xF000 + 0x30*4096 + k); break;
    }
}

// ---------------------------------------------------------------------
// Процедуры для работы с выводом текста
// ---------------------------------------------------------------------

// Печать на экране Char
void APP::print16char(int col, int row, unsigned char ch, uint cl) {

    col *= 8;
    row *= 16;

    for (int i = 0; i < 16; i++) {

        unsigned char hl = ansi16[ch][i];
        for (int j = 0; j < 8; j++) {
            if (hl & (1<<(7-j)))
                pset(j + col, i + row, cl);
        }
    }
}

// Печать строки
void APP::print(int col, int row, const char* s, uint cl) {

    int i = 0;
    while (s[i]) { print16char(col, row, s[i], cl); col++; i++; }
}

// ---------------------------------------------------------------------
// Базовые методы вывода графики
// ---------------------------------------------------------------------

// Нарисовать точку
void APP::pset(int x, int y, uint color) {

    if (x >= 0 && y >= 0 && x < width && y < height) {
        ( (Uint32*)sdl_screen->pixels )[ x + width*y ] = color;
    }
}

// Очистить экран в цвет
void APP::cls(uint color) {

    for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
        pset(j, i, color);
}

// Обменять буфер
void APP::flip() {
    SDL_Flip(sdl_screen);
}
