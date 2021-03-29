#include <avr/interrupt.h>
#include <screen3.cc>
#include <keyboard.cc>

screen3  D;
keyboard K;

int main() {

    D.init();
    D.cls();
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");

    for(;;) {
        D.print_char( K.getch() );
    }
}
