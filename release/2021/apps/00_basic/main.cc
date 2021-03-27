#include <avr/interrupt.h>
#include <screen.cc>

screen D;

int main() {

    D.init(7);
    D.locate(0, 0);

    for (;;) {
    }
}
