#include <screen13.cc>

screen13 G;

int mandel(float cx, float cy) {

    float x = cx, y = cy;
    for (int i = 0; i < 256; i++) {

        if (x*x + y*y >= 2) return i;

        float _x = x*x - y*y + cx;
        float _y = 2*x*y     + cy;

        x = _x; y = _y;
    }

    return 0;
}

int main() {

    G.init();

    const float
        f = 1.0 / 320.0,
        x0 = -1.0, y0 = 0;

    for (int y = -100; y < 100; y++)
    for (int x = -160; x < 160; x++) {

        G.pset(160+x, 100+y, mandel(x0 + (float)x*f, y0 + (float)y*f));
    }

    for (;;);
}
