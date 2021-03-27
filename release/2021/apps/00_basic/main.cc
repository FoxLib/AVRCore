#include <avr/interrupt.h>
#include <screen.cc>
#include <keyboard.cc>

screen   D;
keyboard K;

int main() {

    D.init(7);
    D.locate(0, 0);
    D.print("Hello, world: ");

    for (;;) {
        D.print_char(K.getch());
    }
}
