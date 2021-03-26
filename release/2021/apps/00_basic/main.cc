#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>
#include <graphics.cc>

// Текстовый режим
screen   D;
keyboard K;
graphics G;

// Timer :: sei();
int timer; ISR(INT0_vect) { D.locate(2, 3); D.print(timer++); }

// Текстовый режим
void text() {

    D.init(7);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");

    for(;;) {
        D.print_char( K.getch() );
    }
}

// Графический режим
void graph() {

    G.screen(2);

    heap(vm, 0xf000);
    for (int b = 0; b < 0x30; b++) {

        bank(b);
        for (int k = 0; k < 4096; k++)
            vm[k] = k;
    }

    for(;;);
}

int main() {
    graph();
}
