#include <vector3.cc>
#include <stdio.cc>
#include <avr8k/graphics.cc>

#define MAX_STAR 64

graphics D;
stdio  io;

vec3i star[MAX_STAR];
const int ppd = 100;

int main() {

    D.cls(0);

    for (int i = 0; i < MAX_STAR; i++) {

        star[i].x = io.rand()%320 - 160;
        star[i].y = io.rand()%320 - 160;
        star[i].z = io.rand()%250 + 10;
    }

    for(;;) {

        for (int i = 0; i < MAX_STAR; i++) {

            // Удалить точку
            int x = 160 + (long)star[i].x * ppd / star[i].z;
            int y = 100 + (long)star[i].y * ppd / star[i].z;
            D.pset(x, y, 0);

            // Смещение
            star[i].z -= 2;

            if (star[i].z <= 1) {
                star[i].z = 260;
            }

            // Перевычислить
            x = 160 + (long)star[i].x * ppd / star[i].z;
            y = 100 + (long)star[i].y * ppd / star[i].z;

            int cl = 8;
            if (star[i].z < 200) cl = 7;
            else if (star[i].z < 100) cl = 15;

            D.pset(x, y, cl);

        }

    }
}
