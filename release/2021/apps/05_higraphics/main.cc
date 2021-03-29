#include <avr/interrupt.h>
#include <graphics.cc>
#include <math.h>

graphics G;

int main() {

    G.screen12();
    G.cls(1);

    byte k = 0;
    for (;;) {

        for (int y = 0; y < 400; y++)
        for (int x = 0; x < 640; x++)
            G.pixel(x, y, x+y+k);

        k++;
    }
}
