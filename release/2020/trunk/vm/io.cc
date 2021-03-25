#include "main.h"

// ---------------------------------------------------------------------
// РАБОТА С ПОРТАМИ
// ---------------------------------------------------------------------

// Чтение из памяти
unsigned char APP::get(int addr) {

    addr &= 0xFFFF;

    // Смена банков памяти
    if (addr >= 0xF000) addr += (bank * 0x1000);

    unsigned char data = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Управление клавиатурой
        case 0x20: data = spi_cmd; break;
        case 0x21: data = spi_st; break; // Busy=0, Timeout=0
        case 0x22: data = 1; break;      // DRAM Ready=1
        case 0x2C: data = port_keyb_xt; break;
        case 0x2D: data = port_kb_cnt; break;
        case 0x2E: data = timer & 0xFF; break;
        case 0x2F: data = timer >> 8; break;
        case 0x30: data = bank; break;
        case 0x31: data = videom; break;
        case 0x32: data = cursor_x; break;
        case 0x33: data = cursor_y; break;
        case 0x3E: data = dynamic_ram[dram_address & 0x3FFFFFF]; break;
        case 0x4E: data = spi_data; break;

        // Остальная память
        default: data = sram[addr]; break;
    }

    return data & 0xFF;
}

// Сохранение в память
void APP::put(int addr, unsigned char data) {

    addr &= 0xFFFF;
    if (addr >= 0xF000) addr += (bank * 0x1000);

    // Сохрание в память
    sram[addr] = data;

    switch (addr) {

        // DRAM
        case 0x2C: dram_address = (dram_address & ~0x000000FF) | data;         break;
        case 0x2D: dram_address = (dram_address & ~0x0000FF00) | (data << 8);  break;
        case 0x2E: dram_address = (dram_address & ~0x00FF0000) | (data << 16); break;
        case 0x2F: dram_address = (dram_address & ~0xFF000000) | (data << 24); break;

        // Установка банка памяти
        case 0x30: bank = data; break;

        // Изменение видеорежима
        case 0x31:

            videom = data;

            if (videom == 1) { width = 2*640; height = 2*480; }
            else             { width = 2*640; height = 2*400; }

            SDL_FreeSurface(sdl_screen);
            sdl_screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

            update_screen();
            break;

        // Установка положения курсора
        case 0x32: cursor_update(); cursor_x = data; cursor_update(); break;
        case 0x33: cursor_update(); cursor_y = data; cursor_update(); break;

        // SPI Data
        case 0x4E: spi_data = data; break;
        case 0x20: spi_cmd  = data; break;
        case 0x21: if ((data & 1) && (spi_latch == 0)) { spi_write_cmd(data); } spi_latch = data & 1; break;

        // DRAM
        case 0x22: if (data & 1) dynamic_ram[dram_address & 0x3FFFFFF] = data; break;
        case 0x3E: dram_data = data; break;

        // Запись во флаги
        case 0x5F: byte_to_flag(data); break;
    }

    // Нарисовать на холсте
    if (addr >= 0xF000) update_byte_scr(addr);
}

// ---------------------------------------------------------------------
// РАБОТА С ЭКРАНОМ
// ---------------------------------------------------------------------

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

            if (addr < 4000) {

                update_text_xy(prev_x, prev_y);
                update_text_xy(x, y);
                prev_x = x;
                prev_y = y;
            }

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
