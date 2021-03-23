#include <avr/interrupt.h>

#include <avrio.c>
#include <textmode.c>

volatile char* m = (char*)0xF000;

// Назначение обработчка
ISR(INT0_vect) {

    m[0]++;
}

int main() {

    cls(0x17);
    outp(TIMER_INTR, 25);
    sei();
    for(;;);
}
