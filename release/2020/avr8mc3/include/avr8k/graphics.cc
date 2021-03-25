#include <screen.cc>

class graphics : public screen {
protected:

    word cursor_x, cursor_y;
    char color_fr, color_bg;

public:

    // Очистить экран
    void cls(byte cl) {

        DISPLAY(vm);
        cl = (cl << 4) | cl;
        for (int i = 0; i < 32000; i++)
            vm[i] = cl;
    }

    // Рисование точки на экране
    void pset(int x, int y, char cl) {

        DISPLAY(vm);

        // Не должен превышать границы
        if (x < 0 || y < 0 || x >= 320 || y >= 200)
            return;

        word z = y*160 + (x>>1); // Расчет номера байта
        cl &= 15;

        // Установка точки в ниббл
        vm[z] = x & 1 ? ((vm[z] & 0xF0) | cl) : ((vm[z] & 0x0F) | (cl << 4));
    }

    // Вернуть точку
    byte point(int x, int y) {

        DISPLAY(vm);
        word z = y*160 + (x>>1); // Расчет номера байта
        return x & 1 ? vm[z] & 0x0F : (vm[z] >> 4);
    }

    // Ускоренное рисование блока
    void block(int x1, int y1, int x2, int y2, byte cl) {

        DISPLAY(vm);

        // Выход за пределы рисования
        if (x2 < 0 || y2 < 0 || x1 >= 320 || y1 >= 200) return;
        if (x1 < 0) x1 = 0; if (x2 > 319) x2 = 319;
        if (y1 < 0) y1 = 0; if (y2 > 199) x2 = 199;

        // Расчет инициирующей точки
        word  xc = (x2>>1) - (x1>>1);     // Расстояние
        word  cc = cl | (cl << 4);        // Сдвоенный цвет
        word  z  = 160*y1 + (x1>>1), zc;

        // Коррекции, если не попадает
        if (x1 & 1) { z++; xc--; }
        if (x2 & 1) { xc++; }

        // Построение линии сверху вниз
        for (int i = y1; i <= y2; i++) {

            // Рисование горизонтальной линии
            zc = z; for (word j = 0; j < xc; j++) vm[zc++] = cc;

            // К следующему Y++
            z += 160;
        }

        // Дорисовать линии слева и справа
        if ( (x1 & 1)) for (int i = y1; i <= y2; i++) pset(x1, i, cl);
        if (!(x2 & 1)) for (int i = y1; i <= y2; i++) pset(x2, i, cl);
    }

    // Рисование тайла на экране (X кратен 2), нет проверки границ
    void tile(const byte* data, int x, int y, int w, int h) {

        DISPLAY(vm);

        w >>= 1;

        word z = 160*y + (x>>1), zc;
        int  n = 0;

        // Построчное рисование
        for (int i = 0; i < h; i++) {

            zc = z;
            for (int j = 0; j < w; j++) vm[zc++] = pgm_read_byte(&data[n++]);
            z += 160;
        }

    }
};
