#include "main.h"

int main(int argc, char* argv[]) {

    APP avr("Виртуальная машина AVR");
    if (argc > 1) avr.loadfile(argv[1]);

    return avr.start();
}

