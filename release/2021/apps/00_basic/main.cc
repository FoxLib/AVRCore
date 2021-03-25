#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>

// Текстовый режим
screen   D;
keyboard K;

// Timer
int timer; ISR(INT0_vect) { D.locate(2, 3); D.print(timer++); }

int main() {

    D.init();
    D.cls(0x17);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");

    // sei();

    for(;;) {

        D.print_char( K.getch() );

        /*
        while (((inp(0x02)^pk)&1) == 0); pk = inp(0x02);
        int k = inp(0x01);
        if (!(k & 0x80)) D.print_char((byte)k);
        */
    }
}
