#include "screen.cc"

struct vec2 { float x, y; };
struct vec3 { float x, y, z; };

class screen13 : public screen {
protected:

    byte bankbase; // 0x20 или 0x30

public:

    // -----------------------------------------------------------------
    // Реализация виртуальных методов
    // -----------------------------------------------------------------

    void init() { screen(0); }
    void cls()  { cls(0); }
    void print_char(byte x, byte y, byte ch) { }

    // -----------------------------------------------------------------
    // Методы работы с графикой
    // -----------------------------------------------------------------

    void screen(byte _mode) {

        bankbase = _mode ? 0x30 : 0x20;
        outp(VIDEOMODE, 2 + _mode);
    }

    // Очистить экран
    void cls(byte c) {

        heapvm;
        for (int a = 0; a < 16; a++) {
            bank(bankbase + a);
            for (int b = 0; b < 4096; b++) {
                vm[b] = c;
            }
        }
    }

    // Установка точки на экране (320x200)
    void pset(word x, word y, byte cl) {

        heapvm;
        word A = (y<<6) + (y<<8) + x;
        bank(bankbase + (A>>12));
        vm[A & 0xFFF] = cl;
    }

    // 24->8 бит
    byte rgb(byte r, byte g, byte b) { return (r&0xE0) | ((g&0xE0)>>3) | (b>>6); }
};
