#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>
#include <stdlib.h>

screen   D;
keyboard K;

int main() {

    D.init(7);
    D.locate(0, 0);
    D.print("Hello, world: ");
    D.print( rand() );

    for (;;) {
        D.print_char(K.getch());
    }
}
