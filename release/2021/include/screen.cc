#include "avrio.cc"
#include "format.cc"

/** @desc Класс для работы с текстовым видеорежимом
 */

class screen {
protected:

    byte cursor_x, cursor_y, cursor_cl;
    format o_format;

public:

    void init(int attr) {

        bank(2);
        outp(VIDEOMODE, 0);
        locate(0, 0);
        cursor_cl = 0x07;

        if (attr > 0) cls(attr);
    }

    void init() { init(-1); }

    // -----------------------------------------------------------------
    // Функции для работы с видеобуфером
    // -----------------------------------------------------------------

    // Очистить экран
    void cls() { cls(cursor_cl, 0x20); }
    void cls(byte attr) { cls(attr, 0x20); }
    void cls(byte attr, byte ch) {

        heap(vm, 0xF000);
        for (int i = 0; i < 4000; i += 2) {
            vm[i]   = ch;
            vm[i+1] = attr;
        }
        cursor_cl = attr;
    }

    // Переместить курсор
    void locate(byte x, byte y) {

        if (inp(CURSOR_Y) < 25) {
            outp(CURSOR_X, x);
            outp(CURSOR_Y, y);
        }

        cursor_x = x;
        cursor_y = y;
    }

    // Скрыть или показать курсор
    void hide() { outp(CURSOR_Y, 25); }
    void show() { outp(CURSOR_Y, cursor_y); }

    // Обновить палитру
    void palette(byte id, byte r, byte g, byte b) {

        heap(vm, 0xFFA0);

        vm[2*id + 1] = (r >> 4);
        vm[2*id + 0] = (g & 0xF0) | (b >> 4);
    }

    // -----------------------------------------------------------------
    // Печать символов
    // -----------------------------------------------------------------

    // Печать символа на экране
    void print_char(byte x, byte y, char ch) {

        heap(vm, 0xF000);
        int   z = (x<<1) + (y<<7) + (y<<5);

        vm[z]   = ch;
        vm[z+1] = cursor_cl;
    }

    // Печать в режиме телетайпа
    void print_char(byte s) {

        int i;
        heap(vm, 0xF000);

        if (s == 10) {
            cursor_x = 80;
        } else {
            print_char(cursor_x, cursor_y, s);
            cursor_x++;
        }

        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }

        // Скроллинг вверх
        if (cursor_y >= 25) {

            for (i = 0; i < 4000 - 160; i += 2) {
                vm[i]   = vm[i + 160];
                vm[i+1] = vm[i + 161];
            }

            // Очистка новой строки
            for (i = 4000 - 160; i < 4000; i += 2) {
                vm[i]   = ' ';
                vm[i+1] = cursor_cl;
            }

            cursor_y = 24;
        }

        locate(cursor_x, cursor_y);
    }

    // Печать строки
    int print(const char* s) {

        int i = 0;

        while (s[i]) {
            print_char(s[i++]);
        }

        return i;
    }

    // Печать числа
    int print(long v) {

        o_format.i2a(v);
        return print((const char*) o_format.buffer);
    }
};
