#include "avr.h"

int main(int argc, char* argv[]) {

    APP avr;

    avr.config();
    avr.window("AVR emulator");
    avr.assign();

    if (argc > 1) { avr.loadfile(argv[1]); }

    avr.ds_update();
    avr.infinite();

    return 0;
}
