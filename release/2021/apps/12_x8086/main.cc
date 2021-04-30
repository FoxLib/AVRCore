#include <avr/pgmspace.h>
#include <screen3.cc>
#include <dram.cc>

// Текстовый видеорежим
screen3 D;
dram    M;

#include "bios.h"
#include "iface.cc"
#include "portable86.cc"

int main() {

    D.init();
    D.cls(7);
    initcpu();

    step();

    for (;;);
}
