#include "avrio.cc"
#include "screen.cc"

/** @desc Класс для работы с текстовым видеорежимом
 */

class screen3 : public screen {
protected:
public:

    // -----------------------------------------------------------------
    // Реализация виртуальных методов
    // -----------------------------------------------------------------

    // Запуск видеорежима
    void init() {

        bank(2);
        outp(VIDEOMODE, 0);

        cursor_cl = 0x07;
        width     = 80;
        height    = 25;
        locate(0, 0);
    }

    // Очистка экрана
    void cls() { cls(cursor_cl, 0x20); }

    // Реализация второго вида виртуального метода
    void print_char(byte s) { screen::print_char(s); }

    // Печать символа на экране
    void print_char(byte x, byte y, byte ch) {

        heapvm;
        bank(2);

        int z = (x<<1) + (y<<7) + (y<<5);
        vm[z]   = ch;
        vm[z+1] = cursor_cl;
    }

    // Переместить курсор
    void locate(byte x, byte y) {

        if (inp(CURSOR_Y) < 25) {
            outp(CURSOR_X, x);
            outp(CURSOR_Y, y);
        }

        screen::locate(x, y);
    }

    // Скроллинг экрана
    void scrollup() {

        heapvm;
        bank(2);

        int i;
        for (i = 0; i < 4000 - 160; i += 2) {
            vm[i]   = vm[i + 160];
            vm[i+1] = vm[i + 161];
        }

        // Очистка новой строки
        for (i = 4000 - 160; i < 4000; i += 2) {
            vm[i]   = ' ';
            vm[i+1] = cursor_cl;
        }
    }

    // -----------------------------------------------------------------
    // Функции работы с видеобуфером
    // -----------------------------------------------------------------

    // Очистить экран
    void cls(byte attr) { cls(attr, 0x20); }
    void cls(byte attr, byte ch) {

        heapvm;
        init();

        for (int i = 0; i < 4000; i += 2) {
            vm[i]   = ch;
            vm[i+1] = attr;
        }

        cursor_cl = attr;
    }

    // Скрыть или показать курсор
    void hide() { outp(CURSOR_Y, 25); }
    void show() { outp(CURSOR_Y, cursor_y); }

    // Обновить палитру
    void palette(byte id, byte r, byte g, byte b) {

        heapvm;
        bank(2);

        vm[2*id + 1] = (r >> 4);
        vm[2*id + 0] = (g & 0xF0) | (b >> 4);
    }
};
