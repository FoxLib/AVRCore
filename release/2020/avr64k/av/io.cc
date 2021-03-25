#include "main.h"

// Чтение из памяти
unsigned char APP::get(int addr) {

    addr &= 0xFFFF;
    if (addr >= 0xF000) addr += (bank * 0x1000);

    unsigned char dv = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Управление клавиатурой
        case 0x20: dv = spi_cmd; break;
        case 0x21: dv = spi_st; break; // Busy=0, Timeout=0
        case 0x22: dv = 1; break;      // DRAM Ready=1
        case 0x2C: dv = port_keyb_xt; break;
        case 0x2D: dv = port_kb_cnt; break;
        case 0x2E: dv = timer & 0xFF; break;
        case 0x2F: dv = timer >> 8; break;
        case 0x30: dv = bank; break;
        case 0x31: dv = videom; break;
        case 0x32: dv = cursor_x; break;
        case 0x33: dv = cursor_y; break;
        case 0x3E: dv = dynamic_ram[dram_address & 0x3FFFFFF]; break;
        case 0x4E: dv = spi_data; break;

        // Остальная память
        default: dv = sram[addr]; break;
    }

    return dv & 0xFF;
}

// Сохранение в память
void APP::put(int addr, unsigned char value) {

    addr &= 0xFFFF;
    if (addr >= 0xF000) addr += (bank * 0x1000);

    // Сохрание в память
    sram[addr] = value;

    switch (addr) {

        // DRAM
        case 0x2C: dram_address = (dram_address & ~0x000000FF) | value;         break;
        case 0x2D: dram_address = (dram_address & ~0x0000FF00) | (value << 8);  break;
        case 0x2E: dram_address = (dram_address & ~0x00FF0000) | (value << 16); break;
        case 0x2F: dram_address = (dram_address & ~0xFF000000) | (value << 24); break;

        // Установка банка памяти
        case 0x30: bank = value; break;

        // Изменение видеорежима
        case 0x31:

            videom = value;

            if (videom == 1) { width = 2*640; height = 2*480; }
            else             { width = 2*640; height = 2*400; }

            SDL_FreeSurface(sdl_screen);
            sdl_screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

            update_screen();
            break;

        // Установка положения курсора
        case 0x32: cursor_update(); cursor_x = value; cursor_update(); break;
        case 0x33: cursor_update(); cursor_y = value; cursor_update(); break;

        // SPI Data
        case 0x4E: spi_data = value; break;
        case 0x20: spi_cmd  = value; break;
        case 0x21: if ((value & 1) && (spi_latch == 0)) { spi_write_cmd(value); } spi_latch = value & 1; break;

        // DRAM
        case 0x22: if (value & 1) dynamic_ram[dram_address & 0x3FFFFFF] = value; break;
        case 0x3E: dram_data = value; break;

        // Запись во флаги
        case 0x5F: byte_to_flag(value); break;
    }

    // Нарисовать на холсте
    if (addr >= 0xF000) update_byte_scr(addr);
}

// Получение ASCII
int APP::get_key_ascii(SDL_Event event) {

    /* Получение ссылки на структуру с данными о нажатой клавише */
    SDL_KeyboardEvent * eventkey = & event.key;

    int xt = 0;
    int k = eventkey->keysym.scancode;

    switch (k) {

        /* A */ case 0x26: xt = 0x41; break;
        /* B */ case 0x38: xt = 0x42; break;
        /* C */ case 0x36: xt = 0x43; break;
        /* D */ case 0x28: xt = 0x44; break;
        /* E */ case 0x1a: xt = 0x45; break;
        /* F */ case 0x29: xt = 0x46; break;
        /* G */ case 0x2a: xt = 0x47; break;
        /* H */ case 0x2b: xt = 0x48; break;
        /* I */ case 0x1f: xt = 0x49; break;
        /* J */ case 0x2c: xt = 0x4A; break;
        /* K */ case 0x2d: xt = 0x4B; break;
        /* L */ case 0x2e: xt = 0x4C; break;
        /* M */ case 0x3a: xt = 0x4D; break;
        /* N */ case 0x39: xt = 0x4E; break;
        /* O */ case 0x20: xt = 0x4F; break;
        /* P */ case 0x21: xt = 0x50; break;
        /* Q */ case 0x18: xt = 0x51; break;
        /* R */ case 0x1b: xt = 0x52; break;
        /* S */ case 0x27: xt = 0x53; break;
        /* T */ case 0x1c: xt = 0x54; break;
        /* U */ case 0x1e: xt = 0x55; break;
        /* V */ case 0x37: xt = 0x56; break;
        /* W */ case 0x19: xt = 0x57; break;
        /* X */ case 0x35: xt = 0x58; break;
        /* Y */ case 0x1d: xt = 0x59; break;
        /* Z */ case 0x34: xt = 0x5A; break;

        /* 0 */ case 0x13: xt = 0x30; break;
        /* 1 */ case 0x0A: xt = 0x31; break;
        /* 2 */ case 0x0B: xt = 0x32; break;
        /* 3 */ case 0x0C: xt = 0x33; break;
        /* 4 */ case 0x0D: xt = 0x34; break;
        /* 5 */ case 0x0E: xt = 0x35; break;
        /* 6 */ case 0x0F: xt = 0x36; break;
        /* 7 */ case 0x10: xt = 0x37; break;
        /* 8 */ case 0x11: xt = 0x38; break;
        /* 9 */ case 0x12: xt = 0x39; break;

        /* ` */ case 0x31: xt = 0x60; break;
        /* - */ case 0x14: xt = 0x2D; break;
        /* = */ case 0x15: xt = 0x3D; break;
        /* \ */ case 0x33: xt = 0x5C; break;
        /* [ */ case 0x22: xt = 0x5B; break;
        /* ] */ case 0x23: xt = 0x5D; break;
        /* ; */ case 0x2f: xt = 0x3B; break;
        /* ' */ case 0x30: xt = 0x27; break;
        /* , */ case 0x3b: xt = 0x2C; break;
        /* . */ case 0x3c: xt = 0x2E; break;
        /* / */ case 0x3d: xt = 0x2F; break;

        /* F1  */ case 67: xt = key_F1; break;
        /* F2  */ case 68: xt = key_F2; break;
        /* F3  */ case 69: xt = key_F3; break;
        /* F4  */ case 70: xt = key_F4; break;
        /* F5  */ case 71: xt = key_F5; break;
        /* F6  */ case 72: xt = key_F6; break;
        /* F7  */ case 73: xt = key_F7; break;
        /* F8  */ case 74: xt = key_F8; break;
        /* F9  */ case 75: xt = key_F9; break;
        /* F10 */ case 76: xt = key_F10; break;
        /* F11 */ case 95: xt = key_F11; break;
        /* F12 */ case 96: xt = key_F12; break;

        /* bs */ case 0x16: xt = key_BS; break;     // Back Space
        /* sp */ case 0x41: xt = 0x20; break;       // Space
        /* tb */ case 0x17: xt = key_TAB; break;    // Tab
        /* ls */ case 0x32: xt = key_LSHIFT; break; // Left Shift
        /* lc */ case 0x25: xt = key_LALT;  break;  // Left Ctrl
        /* la */ case 0x40: xt = key_LCTRL; break;  // Left Alt
        /* en */ case 0x24: xt = key_ENTER; break;  // Enter
        /* es */ case 0x09: xt = key_ESC; break;    // Escape
        /* es */ case 0x08: xt = key_ESC; break;

        // ---------------------------------------------
        // Специальные (не так же как в реальном железе)
        // ---------------------------------------------

        /* UP  */  case 0x6F: xt = key_UP; break;
        /* RT  */  case 0x72: xt = key_RT; break;
        /* DN  */  case 0x74: xt = key_DN; break;
        /* LF  */  case 0x71: xt = key_LF; break;
        /* Home */ case 0x6E: xt = key_HOME; break;
        /* End  */ case 0x73: xt = key_END; break;
        /* PgUp */ case 0x70: xt = key_PGUP; break;
        /* PgDn */ case 0x75: xt = key_PGDN; break;
        /* Del  */ case 0x77: xt = key_DEL; break;
        /* Ins  */ case 0x76: xt = key_INS; break;
        /* NLock*/ case 0x4D: xt = key_NL; break;

        default: return -k;
    }

    /* Получить скан-код клавиш */
    return xt;
}
