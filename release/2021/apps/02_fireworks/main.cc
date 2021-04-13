#include <screen13.cc>
#include <math.h>

screen13 G;

int random_id = 1;

struct pt {
    float x,   y,  z;
    float sx, sy, lt;
};

struct pt fp[320];

word rand() {

    random_id = (random_id * 1103515245) + 12345;
    return random_id;
}

int main() {

    heap(vm, 0xf000);

    random_id = inp(TIMERL);

    G.screen(0);

    for (;;) {

        int nfree = 0;

        // Затухание
        for (int a = 0; a < 16; a++) {
            bank(0x20 + a);
            for (int b = 0; b < 4096; b++) {
                vm[b] = vm[b] > 4 ? vm[b] - 4 : 0;
            }
        }

        // Процессинг
        for (int i = 0; i < 320; i++) {

            if (fp[i].lt <= 0 || fp[i].y > 199 || fp[i].x < 0 || fp[i].x > 319) {
                fp[i].lt = 0;
                nfree++;
            }
            else {

                int m = fp[i].lt;
                if (fp[i].y >= 0) G.pset(fp[i].x, fp[i].y, m);

                fp[i].x  += fp[i].sx;
                fp[i].y  += fp[i].sy;
                fp[i].sy += .01;
                fp[i].sx *= .995;
                fp[i].lt -= .25;
            }

            if (nfree > 160) {

                float k  = 0;
                int rx = rand() % 320,
                    ry = rand() % 200;

                for (int i = 0; i < 320; i++) {

                    if (fp[i].lt == 0) {

                        float v = (float)(rand() % 255) / 512.0 + 0.5;
                        fp[i].x = rx;
                        fp[i].y = ry;
                        fp[i].sx = sin(k) * v;
                        fp[i].sy = cos(k) * v;
                        fp[i].lt = 255 - (rand()%16);
                        k += v;
                    }
                }
            }
        }
    }
}
