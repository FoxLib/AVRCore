#include <keyboard.cc>
#include <screen13.cc>
#include <math.h>

screen13 G;
keyboard K;

int main() {

    G.init();
    G.flip();

    float k = 0;

    for (;;) {

        G.cls(0);
        G.circle_fill(160 + sin(k)*100, 100 + cos(3*k)*50, 16, 50);
        G.flip();

        k += 0.03;
        //K.getch();
    }

    //for (;;);
}
