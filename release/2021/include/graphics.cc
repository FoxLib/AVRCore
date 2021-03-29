#include "avrio.cc"

struct vec2 { float x, y; };
struct vec3 { float x, y, z; };

class graphics {
protected:

    byte bankbase; // 0x20 или 0x30

    // Вычисление банка пикселя для разрешения 640x
    word pixelbank(word x, word y) {

        // Вычисление базового адреса
        dword A = ((dword)y<<6) + ((dword)y<<8) + (x>>1);

        // Скоростная техника A>>12
        bank( (((byte*)&A)[1]>>4) | (((byte*)&A)[2] ? 0x30:0x20) );

        return A & 0xFFF;
    }

public:

    // 320x200: Выбор страницы
    void init() { screen(0); }
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
