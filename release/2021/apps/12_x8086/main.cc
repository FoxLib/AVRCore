#include <avr/pgmspace.h>
#include <screen3.cc>
#include <dram.cc>

/**
 * Распределение памяти:
 *
 * 00000-003FF  IVT  1k
 * 00400-9FFFF  RAM  640k
 * B8000-B8FFF  TXT  4k
 * C0000-CFFFF  BIOS
 */

// Текстовый видеорежим
screen3 D;
dram    M;

const byte BIOS[] PROGMEM = "Program Space\0";

// Работа с памятью (чтение из BIOS/памяти)
uint8_t readmemb(uint16_t S, uint16_t A) {

    // Чтение из BIOS
    if (S >= 0xf000) return pgm_read_byte(&BIOS[(S<<4) + A]);

    // Чтение из памяти
    return M.peek((S<<4) + A);
}

void writememb(uint16_t S, uint16_t A, uint8_t V) { M.poke((S<<4) + A, V); }

#include "portable86.cc"

int main() {

    D.init();
    D.cls(7);

    initcpu();

    for (int i = 0; i < 128; i++) {

        D.print(readmemb(0xf000, i), 16, 1);
        D.print(" ");
    }


    D.print(flags, 16, 2);


    for (;;);
}
