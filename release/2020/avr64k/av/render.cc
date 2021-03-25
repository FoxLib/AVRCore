#include "main.h"

// Обновить весь экран
void APP::display_update() {

    cls(0);

    int wsize = 0;
    switch (videom) {

        case 0: wsize = 4;   break;    // Видеорежим 80x30
        case 1: wsize = 150; break;    // Видеорежим 640x480x4
        case 2: wsize = 64;  break;    // Видеорежим 320x200x256
        case 3: wsize = 128; break;    // Видеорежим 320x200x64k
    }

    for (int i = 0; i < wsize*1024; i++) update_byte_scr(0x10000 + i);
}

// Видеопамять начинается с $10000 (1-я страница)
void APP::update_byte_scr(int addr) {

    // Не рендерить в дебаггере
    if (ds_debugger) return;

    int x, y, gb, r, cl;
    addr -= 0x10000;

    switch (videom) {

        case 0: // TEXT 80x25x16

            x = (addr>>1) % 80;
            y = (addr>>1) / 80;
            if (addr < 4000) update_text_xy(x, y);
            break;

        case 1: // GRAPHICS 640x480x4

            break;

        case 2: // GRAPHICS 320x200x8

            x = addr % 320;
            y = addr / 320;
            cl = get_palette(sram[0x10000 + x + y*320]);

            for (int i = 0; i < 16; i++)
                pset(4*x + (i>>2), 4*y + (i&3), cl);

            break;

        case 3: // GRAPHICS 320x200x16

            break;
    }
}

// Обновить текст в (X, Y)
void APP::update_text_xy(int X, int Y) {

    int k;
    int addr = 0x10000 + 2*(X + Y*80);
    int ch   = sram[ addr + 0 ];
    int attr = sram[ addr + 1 ];

    for (int y = 0; y < 16; y++) {

        int ft = sram[MEMORY_FONT_ADDR + 16*ch + y];
        for (int x = 0; x < 8; x++) {

            int cbit   = ft & (1 << (7 - x));
            int cursor = (cursor_x == X && cursor_y == Y) && (y >= 14) ? 1 : 0;
            int color  = cbit ^ (flash & cursor) ? (attr & 0x0F) : (attr >> 4);
                color  = get_palette(color);

            // 2x2 Размер пикселя
            for (int k = 0; k < 4; k++) pset(2*(8*X + x) + k%2, 2*(16*Y + y) + k/2, color);
        }
    }
}

// Вычисляется цвет из заданной палитры
int APP::get_palette(uint8_t cl) {

    int gb = sram[MEMORY_FONT_PAL + 2*cl    ];
    int  r = sram[MEMORY_FONT_PAL + 2*cl + 1];
    return ((gb & 0x0F) << 4) | ((gb & 0xF0) << 8) | ((r & 0x0F) << 20);
}

// Если видеорежим 0, то обновить курсор
void APP::cursor_update() {

    if (videom == 0) {

        int btc = 0x10000 + 2*cursor_x + cursor_y*160;
        update_byte_scr(btc);
        update_byte_scr(btc+1);
    }
}
