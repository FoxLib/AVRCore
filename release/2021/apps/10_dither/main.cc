#include <screen13.cc>

screen13 G;

int main() {

    G.cls();

    for (int y = 0; y < 200; y++)
    for (int x = 0; x < 320; x++) {
        G.pset(x, y, G.rgb(x, x+y, y));
    }

    for (int y = 0; y < 200; y++)
    for (int x = 0; x < 320; x++) {
        G.setpixel(x, y, x, x+y, y);
    }

    for (;;);
}
