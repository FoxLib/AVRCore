#include <avr/interrupt.h>
#include <screen.cc>
#include <sd.cc>

screen D;
SD     sd;

int main() {

    D.init(7);

    D.print(" error: "); D.print( sd.init() );
    D.print(" type: ");  D.print( sd.get_sd_type() );

    for (;;) { }
}
