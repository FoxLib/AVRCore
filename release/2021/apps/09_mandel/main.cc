#include <screen13.cc>

screen13 G;

int mandel(float cx, float cy) {

    float x = cx, y = cy;

    for (int i = 0; i < 256; i++) {

        float _x = x*x - y*y + cx;
        float _y = 2*x*y     + cy;

        if (x*x + y*y >= 2) return i;

        x = _x; y = _y;
    }

    return 0;
}

int main() {

    G.init();

    const float f = 1.0/100.0;
    for (int y = -100; y < 100; y++)
    for (int x = -160; x < 160; x++) {

        G.pset(160+x, 100+y, mandel((float)x*f, (float)y*f));
    }

    for (;;);
}
