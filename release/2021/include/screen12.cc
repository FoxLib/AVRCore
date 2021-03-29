#include "graphics.cc"

class graphicshi : public graphics {
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

    // 640x400
    void init() { outp(VIDEOMODE, 1); }

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
    }

    // Установка точки на экране (640x400)
    void pset(word x, word y, byte cl) {

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
};
