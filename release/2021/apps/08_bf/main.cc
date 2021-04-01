#include <avr/pgmspace.h>
#include <screen3.cc>
#include <keyboard.cc>

#define READ(x) pgm_read_byte(&bf[x])

//#include "prg_e.h"
//#include "prg_factorial2.h"
#include "prg_tictac.h"

screen3  D;
keyboard K;

// Чтение и запись байта памяти (64кб)
byte read (word address)             { heapvm; bank(0x20 + (address>>12)); return vm[address & 0xfff]; }
void write(word address, byte value) { heapvm; bank(0x20 + (address>>12)); vm[address & 0xfff] = value; }

int main() {

    // Интерактивный режим (1)
    byte inter = 1;

    D.init();
    D.cls(7);

    byte op, tm;
    int  pc = 0, ad = 0, brc = 0;

    // Считывание следующей команды
    while (op = READ(pc++)) {

        switch (op) {

            // Операторы
            case '+': write(ad, read(ad)+1); break;
            case '-': write(ad, read(ad)-1); break;
            case '>': ad++; break;
            case '<': ad--; break;

            // Ввод-вывод
            case ',': tm = K.getch(); if (tm == 27) tm = 0; if (inter) D.print_char(tm); write(ad, tm); break;
            case '.': D.print_char(read(ad)); break;

            // WHILE
            case '[': if (!read(ad)) {
                brc = 1;
                while (brc && (tm = READ(pc++))) { if (tm == '[') brc++; else if (tm == ']') brc--; }
            }
            break;

            // WEND
            case ']': if (read(ad)) {
                brc = 1;
                pc -= 2;
                while (brc && (tm = READ(pc--))) { if (tm == '[') brc--; else if (tm == ']') brc++; }
                pc += 2;
            }
            break;
        }
    }

    D.color(0x2f);
    D.print(" END ");

    for (;;);
}
