#include <keyboard.cc>
#include <screen13.cc>
#include <math.h>

screen13 G;
keyboard K;

int main() {

    G.init();
    G.flip();

    float r = 30;
    float x = r, y = r, speed_x = 2, speed_y = 0;
    int px = r, py = y, px2 = r, py2 = y;

    for (;;) {

        G.circle_fill(x, y, r, 0);

        x += speed_x;
        y += speed_y;

        if (y >= 199-r) {
            speed_y = -speed_y*0.98;
            y = 199-r;
        }

        if (x >= 319-r) { x = 319-r; speed_x = -speed_x; }
        else if (x < r) { x = r;     speed_x = -speed_x; }

        speed_x *= 0.999;
        speed_y += 0.1;

        // Рисовать шарик
        G.circle_fill(x, y, r, 90);
        G.flip();

        // Стереть шарик, который был нарисован на -2 шаге
        G.circle_fill(px2, py2, r, 0);
        px  = x;  py  = y;
        px2 = px; py2 = py;
    }
}
