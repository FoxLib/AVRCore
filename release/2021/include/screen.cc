#include "avrio.cc"
#include "format.cc"

struct RGB { byte r, b, g; };

class screen {
protected:

    byte   cursor_x, cursor_y, cursor_cl;
    word   width, height;
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
    // @required Скроллинг экрана
    virtual void scrollup();
    // @ommited  Установка курсора
    virtual void locate(byte x, byte y) { cursor_x = x; cursor_y = y; }
    // @ommited  Установка пикселя и блока
    virtual void pset(word x, word y, byte cl) { }
    virtual void block(word x1, word y1, word x2, word y2, byte cl) { }

    // 24->4/8 бит
    virtual byte rgb(byte r, byte g, byte b) { return 0; }

    // -----------------------------------------------------------------

    // Текущий цвет символа
    void color(byte attr) { cursor_cl = attr; }

    // Печать в режиме телетайпа
    virtual void print_char(byte s) {

        if (s == 10) {
            cursor_x = width;
        } else {
            print_char(cursor_x, cursor_y, s);
            cursor_x++;
        }

        if (cursor_x >= width) {
            cursor_x = 0;
            cursor_y++;
        }

        // Скроллинг вверх
        if (cursor_y >= height) {

            scrollup();
            cursor_y = height - 1;
        }

        locate(cursor_x, cursor_y);
    }

    /**
     * @desc Печать строк и так далее на экране
     */

    // Печать строки
    int print(const char* s) {

        int i = 0;
        while (s[i]) print_char(s[i++]);
        return i;
    }

    // Печать числа
    int print(long v) {

        o_format.i2a(v);
        return print((const char*) o_format.buffer);
    }

    // @TODO переделать в d2a
    int print(double v) { return print((float)v); }

    // Печать float-числа
    int print(float v, int n) {

        o_format.f2a(v, n);
        return print((const char*) o_format.buffer);
    }

    // size=1, 2, 4
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

    // Печать строки из Program memory
    int print_pgm(const char* m) {

        int i = 0;
        char ch;
        while (ch = pgm_read_byte(& m[i++])) print_char(ch);
        return i;
    }

    // Алиасы
    int println(const char* s) { byte ln = print(s); print_char(10); return ln; }
    int println(long v)        { byte ln = print(v); print_char(10); return ln; }
    int print(int v) { return print((long)v); }
    int print(dword v) { return print((long)v); }
    int print(float v) { return print(v, 2); }
    int print(long v, byte radix) { return print(v, radix, 1); }

    // =================
    // ГРАФИКА
    // =================

    // Дизеринг паттерном
    void setpixel(word x, word y, byte _r, byte _g, byte _b) {

        int lookup[8][8] =
        {
            { 0, 32,  8, 40,  2, 34, 10, 42},
            {48, 16, 56, 24, 50, 18, 58, 26},
            {12, 44,  4, 36, 14, 46,  6, 38},
            {60, 28, 52, 20, 62, 30, 54, 22},
            { 3, 35, 11, 43,  1, 33,  9, 41},
            {51, 19, 59, 27, 49, 17, 57, 25},
            {15, 47,  7, 39, 13, 45,  5, 37},
            {63, 31, 55, 23, 61, 29, 53, 21},
        };

        // Размытие
        int r = _r + lookup[y&7][x&7] - 32;
        int g = _g + lookup[y&7][x&7] - 32;
        int b = _b + lookup[y&7][x&7] - 32;

        // Учесть ограничения
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;

        pset(x, y, rgb(r, g, b));
    }

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

    // Рисование окружности
    void circle_fill(int xc, int yc, int r, byte c) {

        int x = 0;
        int y = r;
        int d = 3 - 2*y;

        while (x <= y) {

            block(xc-x, yc-y, xc+x, yc-y, c);
            block(xc-x, yc+y, xc+x, yc+y, c);
            block(xc-y, yc-x, xc+y, yc-x, c);
            block(xc-y, yc+x, xc+y, yc+x, c);

            d += 4*x + 6;
            if (d >= 0) {
                d += 4*(1-y);
                y--;
            }

            x++;
        }
    }

};
