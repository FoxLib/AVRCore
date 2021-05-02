#include <avr/pgmspace.h>
#include <screen13.cc>

#include "48k.h"
#include "z80.cc"
#include "zxspectrum.cc"

int main() {

    zxspectrum z80;

    for (;;) {
        z80.frame();
    }
}
