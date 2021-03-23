#include "avrdbg.h"

// ---------------------------------------------------------------------
// РАБОТА С ПОРТАМИ
// ---------------------------------------------------------------------

// Чтение из памяти
unsigned char AVRDBG::get(int addr) {

    addr &= 0xFFFF;
    unsigned char data = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Остальная память
        default: data = sram[addr]; break;
    }

    return data & 0xFF;
}

// Сохранение в память
void AVRDBG::put(int addr, unsigned char data) {

    addr &= 0xFFFF;

    // Сохрание в память
    sram[addr] = data;

    switch (addr) {

        // Запись во флаги
        case 0x5F: byte_to_flag(data); break;
    }

    // Рисование
    // if (addr >= 0xF000) update_byte_scr(addr);
}
