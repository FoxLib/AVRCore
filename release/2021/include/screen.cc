#include "avrio.cc"
#include "format.cc"

class screen {
protected:

    byte   cursor_x, cursor_y, cursor_cl;
    format o_format;

public:

    // Интерфейс
    // -----------------------------------------------------------------
    // @required Запуск видеорежима
    virtual void init();
    // @required Очистка экрана
    virtual void cls();
    // @required Печать символа
    virtual void print_char(byte x, byte y, byte ch);
    // @ommited  Установка курсора
    virtual void locate(byte x, byte y) { cursor_x = x; cursor_y = y; }
    // @ommited  Установка пикселя
    virtual void pset(word x, word y, byte cl) { }
    // -----------------------------------------------------------------

    // Текущий цвет символа
    void color(byte attr) {
        cursor_cl = attr;
    }

    // Печать в режиме телетайпа
    void print_char(byte s) {

        heapvm;
        int i;

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

    int println(const char* s) { byte ln = print(s); print_char(10); return ln; }
    int println(long v)        { byte ln = print(v); print_char(10); return ln; }

    // size=1, 2, 4
    int print(long v, byte radix) { return print(v, radix, 1); }
    int print(long v, byte radix, int size) {

        if (radix == 16) {

            for (int i = 0; i < 2*size; i++) {
                byte nibble = (v >> ((8*size-4) - 4*i)) & 15;
                print_char(nibble + (nibble < 10 ? '0' : '7'));
            }

            return 2*size;
        }

        return 0;
    }

    // =================
    // ГРАФИКА
    // =================

    // Рисование линии
    void line(int x1, int y1, int x2, int y2, byte cl) {

        if (y2 < y1) {
            x1 ^= x2; x2 ^= x1; x1 ^= x2;
            y1 ^= y2; y2 ^= y1; y1 ^= y2;
        }

        int deltax = x2 > x1 ? x2 - x1 : x1 - x2;
        int deltay = y2 - y1;
        int signx  = x1 < x2 ? 1 : -1;

        int error2;
        int error = deltax - deltay;

        while (x1 != x2 || y1 != y2)
        {
            pset(x1, y1, cl);
            error2 = error * 2;

            if (error2 > -deltay) {
                error -= deltay;
                x1 += signx;
            }

            if (error2 < deltax) {
                error += deltax;
                y1 += 1;
            }
        }

        pset(x1, y1, cl);
    }

    // Рисование окружности
    void circle(int xc, int yc, int r, byte c) {

        int x = 0;
        int y = r;
        int d = 3 - 2*y;

        while (x <= y) {

            // --
            pset(xc - x, yc + y, c);
            pset(xc + x, yc + y, c);
            pset(xc - x, yc - y, c);
            pset(xc + x, yc - y, c);
            pset(xc + y, yc + x, c);
            pset(xc - y, yc + x, c);
            pset(xc + y, yc - x, c);
            pset(xc - y, yc - x, c);
            // ...

            d += 4*x + 6;
            if (d >= 0) {
                d += 4*(1 - y);
                y--;
            }

            x++;
        }
    }

};
