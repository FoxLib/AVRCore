#include "avr.h"

int main(int argc, char* argv[]) {

    APP avr;

    if (argc > 1) { avr.loadfile(argv[1]); }

    avr.ds_update();
    avr.main();

    return 0;
}
