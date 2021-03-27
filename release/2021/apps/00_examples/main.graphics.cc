#include <graphics.cc>

graphics G;

int main() {

    G.screen(0);
    byte k = 0;

    for(;;) {

        for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++)
            G.pset(x, y, x + y + k);

        k++;
    }
}
