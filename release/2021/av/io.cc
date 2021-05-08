#include "avr.h"
#include "spi.cc"

// ---------------------------------------------------------------------
// ЧТЕНИЕ В ПАМЯТЬ И ПОРТЫ
// ---------------------------------------------------------------------

// Чтение из памяти
unsigned char APP::get(int addr) {

    addr &= 0xFFFF;

    // Банкинг памяти
    if (addr >= 0xF000) addr += 4096 * membank;

    unsigned char dv = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Банк памяти
        case 0x20: return membank;

        // Клавиатура
        case 0x21: return port_keyb_xt;

        // Статус устройств
        // 0: keyboard hit; 5: spi busy; 6: dram busy; 7: sdram we=1
        case 0x22: return port_keyb_hit | (sdram_ctl & 0x80);

        // Курсор
        case 0x2C: dv = cursor_x; break;
        case 0x2D: dv = cursor_y; break;

        // Таймер
        case 0x2E: dv = timer & 0xff; break;
        case 0x2F: dv = (timer >> 8) & 0xff; break;

        // SDRAM
        case 0x30: return  sdram_addr & 0xff;
        case 0x31: return (sdram_addr >> 8) & 0xff;
        case 0x32: return (sdram_addr >> 16) & 0xff;
        case 0x33: return (sdram_addr >> 24) & 0xff;
        case 0x34: return sdram_data[sdram_addr & 0x3ffffff];

        // Видео
        case 0x38: return videomode;

        // SPI
        case 0x39: dv = spi_read_data(); break;
        case 0x3A: dv = spi_read_status(); break;

        // Остальная память
        default:   dv = sram[addr]; break;
    }

    return dv & 0xFF;
}

// Сохранение в память
void APP::put(int addr, unsigned char value) {

    addr &= 0xFFFF;

    // Учитывать банкинг
    if (addr >= 0xF000) addr += 4096 * membank;

    sram[addr] = value;

    switch (addr) {

        // Системные
        case 0x20: membank = value; break;
        case 0x21: intr_timer = value; break;

        // Флаг записи в память SDRAM | KBHit
        case 0x22:

            if (value & 0x01)
                port_keyb_hit = 0;

            if (value & 0x80)
                sdram_data[sdram_addr & 0x3ffffff] = sdram_data_byte;

            sdram_ctl = value;
            break;

        // Курсор
        case 0x2C:
        case 0x2D:

            if (addr == 0x2C) cursor_x = value;
            else              cursor_y = value;

            update_text_xy(text_px,  text_py);
            update_text_xy(cursor_x, cursor_y);
            text_px = cursor_x;
            text_py = cursor_y;
            break;

        // SDRAM
        case 0x30: sdram_addr = (sdram_addr & 0xFFFFFF00) | value; break;
        case 0x31: sdram_addr = (sdram_addr & 0xFFFF00FF) | (value << 8); break;
        case 0x32: sdram_addr = (sdram_addr & 0xFF00FFFF) | (value << 16); break;
        case 0x33: sdram_addr = (sdram_addr & 0x00FFFFFF) | (value << 24); break;
        case 0x34: sdram_data_byte = value; break;

        // VIDEOMODE
        case 0x38:

            videomode = value;
            require_disp_update = 1;
            break;

        // SPI
        case 0x39: spi_write_data(value); break;
        case 0x3A: spi_write_cmd(value); break;

        // Специальный регистр
        case 0x5F: byte_to_flag(value); break;
    }

    // Нарисовать на холсте
    if (addr >= 0xF000) { update_byte_scr(addr); }
}
