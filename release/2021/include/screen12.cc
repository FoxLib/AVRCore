#include "screen.cc"

const static byte color16[16][3] PROGMEM = {
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x88},
    {0x00, 0x88, 0x00},
    {0x00, 0x88, 0x88},
    {0x88, 0x00, 0x00},
    {0x88, 0x00, 0x88},
    {0x88, 0x88, 0x00},
    {0xcc, 0xcc, 0xcc},
    {0x88, 0x88, 0x88},
    {0x00, 0x00, 0xff},
    {0x00, 0xff, 0x00},
    {0x00, 0xff, 0xff},
    {0xff, 0x00, 0x00},
    {0xff, 0x00, 0xff},
    {0xff, 0xff, 0x00},
    {0xff, 0xff, 0xff},
};

class screen12 : public screen {
protected:

    // Вычисление банка пикселя для разрешения 640x
    word pixelbank(word x, word y) {

        // Вычисление базового адреса
        dword A = ((dword)y<<6) + ((dword)y<<8) + (x>>1);

        // Скоростная техника A>>12
        bank( (((byte*)&A)[1]>>4) | (((byte*)&A)[2] ? 0x30:0x20) );

        return A & 0xFFF;
    }

public:

    // -----------------------------------------------------------------
    // Реализация виртуальных методов
    // -----------------------------------------------------------------

    void init() { outp(VIDEOMODE, 1); color(15); width = 640; height = 400; }
    void cls()  { init(); cls(0); }
    void print_char(byte x, byte y, byte ch) {

        heapvm; bank(3);
        byte font[16];
        word px = x, py = y;

        for (int i = 0; i < 16; i++) font[i] = vm[ch*16 + i];
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                if ((font[i]<<j) & 0x80)
                    pset(px+j, py+i, cursor_cl);
            }
        }
    }

    void scrollup() { }

    // -----------------------------------------------------------------
    // Методы работы с графикой
    // -----------------------------------------------------------------

    void print_char(byte s) {

        if (s == 10) {
            cursor_x = width;
        } else {
            print_char(cursor_x, cursor_y, s);
            cursor_x += 8;
        }

        if (cursor_x >= width) {
            cursor_x = 0;
            cursor_y += 16;
        }
    }

    // Очистить экран
    void cls(byte c) {

        heapvm;

        c = c | (c << 4);
        for (int a = 0x20; a < 0x40; a++) {
            bank(a);
            for (int b = 0; b < 4096; b++) {
                vm[b] = c;
            }
        }

        cursor_cl = 15;
    }

    // Установка точки на экране (640x400)
    void pset(word x, word y, byte cl) {

        if (x >= 640 || y >= 400) return;

        heapvm;
        cl &= 15;

        word A = pixelbank(x, y);
        vm[A] = x&1 ? (cl)|(vm[A]&0xF0) : (cl<<4)|(vm[A]&0x0F);
    }

    // Рисование блока
    void block(word x1, word y1, word x2, word y2, byte cl) {

        heapvm;

        cl &= 15;

        // Расчет инициирующей точки
        word  xc = (x2>>1) - (x1>>1);   // Расстояние
        word  cc = cl | (cl << 4);      // Сдвоенный цвет
        word  z  = pixelbank(x1, y1);   // Начальный банк
        word  zc;

        // Коррекции, если не попадает
        if (x1 & 1) { z++; xc--; }
        if (x2 & 1) { xc++; }

        // Первичный банк памяти
        byte bank = inp(BANK), zb;

        // Построение линии сверху вниз
        for (word i = y1; i <= y2; i++) {

            // Выставить текущий банк
            bank(bank);

            // Сохранение предыдущих указателей
            zc = z;
            zb = bank;

            // Рисование горизонтальной линии
            for (word j = 0; j < xc; j++) {

                vm[zc++] = cc;
                if (zc == 0x1000) { bank(++bank); zc = 0; }
            }
            bank = zb;

            // К следующему Y++
            z += 320;

            // Перелистнуть страницу, если нужно
            if (z >= 0x1000) { z &= 0xFFF; bank++; }
        }

        // Дорисовать линии слева и справа
        if ( (x1 & 1)) for (word i = y1; i <= y2; i++) pset(x1, i, cl);
        if (!(x2 & 1)) for (word i = y1; i <= y2; i++) pset(x2, i, cl);
    }

    // Поиск ближнего цвета к заданному
    byte rgb(byte r, byte g, byte b) {

        byte color = 0;

        long distmin = 4*65536;
        for (int i = 0; i < 16; i++) {

            long r2 = (r - LPM(color16[i][0])),
                 g2 = (g - LPM(color16[i][1])),
                 b2 = (b - LPM(color16[i][2]));

            long dist = r2*r2 + g2*g2 + b2*b2;

            if (dist < distmin) {
                distmin = dist;
                color = i;
            }
        }

        return color;
    }
};
