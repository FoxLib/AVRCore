#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>

screen   D;
keyboard K;

int main() {

    D.init(7);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");

    for(;;) {
        D.print_char( K.getch() );
    }
}
