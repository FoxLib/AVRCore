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

    void init() { screen(0); color(15); width = 40; height = 25; }
    void cls()  { init(); cls(0); }
    void print_char(byte x, byte y, byte ch) { }
    void scrollup() { }

    // -----------------------------------------------------------------
    // Методы работы с графикой
    // -----------------------------------------------------------------

    void screen(byte _mode) {

        bankbase = _mode ? 0x30 : 0x20;
        outp(VIDEOMODE, 2 + _mode);
    }

    // 24->8 бит
    byte rgb(byte r, byte g, byte b) { return (r&0xE0) | ((g&0xE0)>>3) | (b>>6); }

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

        if (x >= 320 || y >= 200) return;

        heapvm;
        word A = (y<<6) + (y<<8) + x;
        bank(bankbase + (A>>12));
        vm[A & 0xFFF] = cl;
    }

    // Рисование блока
    void block(word x1, word y1, word x2, word y2, byte cl) {

        heapvm;

        word A = (y1<<8) + (y1<<6) + x1;
        word B = bankbase + (A>>12);

        A &= 0xFFF;
        for (word y = y1; y <= y2; y++) {

            bank(B);
            word Ap = A;

            // X++
            for (word x = x1; x <= x2; x++) {

                vm[A++] = cl;
                if (A & 0x1000) { bank(inp(BANK)+1); A &= 0x0FFF; }
            }

            // Y++
            A = (Ap + 320);
            if (A & 0x1000) { B++; A &= 0x0FFF; }
        }

    }

    // Обмен буферов
    // Если выбран буфер 0 - пишутся данные в буфер 1, и наоборот
    void flip() {

        byte mode = inp(VIDEOMODE);
        outp(VIDEOMODE, mode ^ 1);
        bankbase = (mode&1) ? 0x30 : 0x20;
    }

};
