#include <avr/pgmspace.h>
#include <screen3.cc>
#include <dram.cc>

screen3 D;
dram    M;

#include "bios.h"
#include "iface.cc"
#include "portable86.cc"
#include "debug.cc"

int main() {

    D.init();
    D.cls(0x07);

    initcpu();

    for (;;) {

        x86run(1024);
        if (inhlt) dumpregs();
    }

}
