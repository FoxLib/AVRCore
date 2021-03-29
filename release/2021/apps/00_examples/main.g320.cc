#include <screen13.cc>

screen13 G;

int main() {

    G.init();

    byte k = 0;
    for(;;) {

        for (int y = 0; y < 200; y++)
        for (int x = 0; x < 160; x++)
            G.pset(x, y, x + y + k);

        G.line(240, 0, 240, 199, 14);
        G.circle(240, 100, 50, 255);

        k++;
    }
}
