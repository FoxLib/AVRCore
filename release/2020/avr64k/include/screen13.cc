#include <avrio.cc>

class Screen13 {
protected:
public:

    Screen13(byte cl) { start(); cls(cl); }

    // Переключение видеорежима
    void start() { videomode(2); }

    // Быстрая очистка экрана в определенный цвет
    void cls(byte cl) {

        heap(vm, 0xf000);
        for (int i = 1; i <= 16; i++) {

            bank(i);
            for (int k = 0; k < 4096; k++) vm[k] = cl;
        }
    }

    // Установка точки
    void pset(int x, int y, byte cl) {

        heap(vm, 0xf000);
        word t = y*320 + x;
        bank((t >> 12) + 1);
        vm[t & 0x0FFF] = cl;
    }

    void block(int x1, int y1, int x2, int y2, byte cl) {
    }

};
