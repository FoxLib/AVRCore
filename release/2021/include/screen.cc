#include "avrio.cc"

/*
 * Класс для работы с текстовым видеорежимом
 */

class screen {
protected:
public:

    screen() { outp(VIDEOMODE, 0); bank(2); }

    // -----------------------------------------------------------------
    // Функции для работы с видеобуфером
    // -----------------------------------------------------------------

    void cls() { cls(0x07, 0x20); }
    void cls(byte attr) { cls(attr, 0x20); }
    void cls(byte attr, byte ch) {

        heap(vm, 0xF000);
        for (int i = 0; i < 4000; i += 2) {
            vm[i]   = ch;
            vm[i+1] = attr;
        }
    }

};
