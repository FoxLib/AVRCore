#include <avr/interrupt.h>
#include <stdlib.h>

#include <screen.cc>
#include <keyboard.cc>
#include <graphicshi.cc>

graphicshi G;
keyboard   K;

int main() {

    G.init();
    G.cls(0);

    for (int y = 0; y < 400; y++)
    for (int x = 0; x < 640; x++)
        G.pset(x, y, (x^y) + ((word)x*(word)y/32));

    /*
    D.init(7);
    D.locate(0, 0);
    D.print("Hello, world: ");
    D.print( rand() );

    for (;;) {
        D.print_char(K.getch());
    }
    * */

    for (;;);
}
