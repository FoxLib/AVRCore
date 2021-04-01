#include <avr/pgmspace.h>
#include <screen3.cc>

// Текстовый видеорежим
screen3 D;

// pgm_read_byte(&program[0])
const byte program[] PROGMEM = "Program Space\0";

int main() {

    D.init();
    D.cls(7);

    for (;;);
}
