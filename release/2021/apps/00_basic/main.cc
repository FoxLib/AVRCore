#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>

// Текстовый режим
screen   D;
keyboard K;

// Timer :: sei();
int timer; ISR(INT0_vect) { D.locate(2, 3); D.print(timer++); }

int main() {

    D.init(7);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");

    for(;;) {

        //D.locate(0, 0);
        //D.print(inp(TIMERL) + (word)256*inp(TIMERH));
        //D.print("   ");

        D.print_char( K.getch() );
    }
}
