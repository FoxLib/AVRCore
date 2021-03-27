#include <avr/interrupt.h>
#include <screen.cc>

screen   D;

int timer; ISR(INT0_vect) {

    D.locate(1, 1);
    D.print(timer++);
}

int main() {

    D.init(7);
    sei();

    for (;;) {

        // D.locate(1, 2);
        // D.print( inp(TIMERL) + inp(TIMERH)*(word)256 );
    }
}
