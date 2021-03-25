#include <avrio.cc>

class SCREEN3 {
protected:

    byte color;

public:

    SCREEN3() { start(); }
    SCREEN3(byte cl) { start(); cls(cl); }

    void start() { videomode(0); }

    // Очистить экран в определенный цвет
    void cls(byte cl) {

        heap(vm, 0xf000);
        bank(1);

        for (int i = 0; i < 4000; i += 2) {

            vm[i+0] = ' ';
            vm[i+1] = cl;
        }

        color = cl;
        locate(0, 0);
    }

    // Печать строки или символа
    void print(const char* s) {

        heap(vm, 0xf000);
        bank(1);

        int i = 0;

        while (s[i]) {

            int cx = inp(CURSOR_X),
                cy = inp(CURSOR_Y);
            int cz = 2*cx + 160*cy;

            vm[cz  ] = s[i];
            vm[cz+1] = color;

            i++;
            cx++;
            if (cx == 80) {
                cx = 0;
                cy++;
                if (cy == 25) {

                    cy = 24;
                    for (int j = 0; j < 4000-160; j++) // Сдвиг вверх
                        vm[j] = vm[j+160];
                    for (int j = 4000-160; j < 4000; j += 2) // Очистка
                        vm[j] = 0x20, vm[j+1] = color;
                }
            }

            locate(cx, cy);
        }

    }
};
