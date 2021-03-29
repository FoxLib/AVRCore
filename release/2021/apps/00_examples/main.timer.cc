#include <avr/interrupt.h>
#include <screen3.cc>

screen3  D;

int timer; ISR(INT0_vect) {

    D.locate(1, 1);
    D.print(timer++);
}

int main() {

    D.init();
    D.cls();

    sei();
    for (;;);
}
