#include <screen12.cc>
#include <stdlib.h>
#include <math.h>

screen12 G;

int main() {

    G.init();
    G.cls();

    float nextx = 0, nexty = 0, x = 0, y = 0;

    for (;;) {

        float rnd = (float) (rand()%32768) / 32768.0;

        if (rnd <= 0.01) {
            nextx = 0; nexty = 0.16*y;

        } else if (rnd <= 0.08) {
            nextx = 0.2  * x - 0.26 * y;
            nexty = 0.23 * x + 0.22 * y + 1.6;

        } else if (rnd <= 0.15) {
            nextx = -.15 * x + .28 * y;
            nexty = .26 * x + .24 * y + .44;
        } else {
            nextx = 0.85 * x + 0.04 * y;
            nexty = -0.04 * x + 0.85 * y + 1.6;
        }

        x = nextx;
        y = nexty;

        G.pset(320 + x*38, 399 - y*38, rand()&1 ? 2 : 10);
    }
}
