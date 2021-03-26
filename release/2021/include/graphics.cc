#include "avrio.cc"

class graphics {
protected:

    byte bankbase; // 0x20 или 0x30

public:

    // 0 или 1 страницы
    void screen(byte _mode) {

        bankbase = _mode ? 0x30 : 0x20;
        outp(VIDEOMODE, 2 + _mode);
    }

    // Установка точки на экране (320x200)
    void pset(word x, word y, byte cl) {

        heap(vm, 0xf000);
        word A = (y<<6) + (y<<8) + x;
        bank(bankbase + (A>>12));
        vm[A & 0xFFF] = cl;
    }

    // 24->8 бит
    byte rgb(byte r, byte g, byte b) { return (r&0xE0) | ((g&0xE0)>>3) | (b>>6); }
};
