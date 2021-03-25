#include "avr.h"

// Чтение из памяти
unsigned char APP::get(int addr) {

    addr &= 0xFFFF;

    unsigned char dv = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Управление клавиатурой
        case 0x20: dv = port_keyb_xt; break;
        case 0x21: dv = port_kb_cnt; break;
        case 0x22: dv = timer & 0xFF; break;
        case 0x23: dv = timer >> 8;   break;

        // Остальная память
        default:   dv = sram[addr]; break;
    }

    return dv & 0xFF;
}

// Сохранение в память
void APP::put(int addr, unsigned char value) {

    addr &= 0xFFFF;

    sram[addr] = value;

    // Запись во флаги
    if (addr == 0x5F) byte_to_flag(value);

    // Нарисовать на холсте
    if (addr >= 0x8000) { update_byte_scr(addr); }
}
