#include "avr.h"

// ---------------------------------------------------------------------
// Отображение видеобуфера на экран
// ---------------------------------------------------------------------

// Обновление экрана
void APP::update_screen() {

    if (ds_debugger)
         ds_update();
    else display_update();
}

// 0x8000 - 0xFFFF Видеопамять
void APP::update_byte_scr(int addr) {

    int xshift = (width  - 640) / 2,
        yshift = (height - 400) / 2;

    // Область видеопамяти
    if (!ds_debugger) {

        addr -= 0x8000;

        // 320x200x4 для Attiny85
        if (cpu_model == ATTINY85) {

            if (addr >= 0 && addr < 32000) {

                int  X = (addr % 160) << 1;
                int  Y = (addr / 160);
                int  cb = sram[0x8000 + addr];

                // 2 Пикселя в байте
                for (int o = 0; o < 2; o++) {

                    uint cl = o ? cb & 15 : (cb >> 4);
                         cl = DOS_13[cl];

                    if (height <= 480) {
                        for (int m = 0; m < 4; m++) {
                            pset((X + o)*2 + (m>>1) + xshift, Y*2 + (m&1) + yshift , cl);
                        }
                    } else {
                        for (int m = 0; m < 16; m++) {
                            pset(4*(X + o) + (m>>2), 4*Y + (m&3), cl);
                        }
                    }
                }
            }
        }
        // Просто модель памяти такая
        else if (cpu_model == ATMEGA328) {

            int s = (width >= 1024 && height >= 800) ? 4 : 2;

            xshift = (width  - s*256) / 2,
            yshift = (height - s*192) / 2;

            if (addr >= 0 && addr < 6144) {

                int  X = (addr & 0x1F);
                int  Y = (addr >> 5);
                int  cb = sram[0x8000 + addr];

                for (int i = 0; i < 8; i++) {

                    int cl = cb & (1 << (7-i)) ? DOS_13[7] : 0;

                    for (int m = 0; m < s*s; m++)
                    pset(xshift + (8*X+i)*s + (m%s), yshift + Y*s + (m/s), cl);
                }
            }
        }
    }
}

// Обновить весь экран
void APP::display_update() {

    cls(0);

    // Видеорежим 320x200x4 или 256x192x1
    for (int i = 0; i < (cpu_model == ATTINY85 ? 32000 : 6144); i++)
        update_byte_scr(0x8000 + i);

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
