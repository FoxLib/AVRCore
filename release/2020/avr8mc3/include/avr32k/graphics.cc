#include <screen.cc>

class graphics : public screen {
protected:
public:

    void cls(char cl) {

        DISPLAY(vm);
        for (int i = 0; i < 6144; i++)
            vm[i] = cl ? 255 : 0;

        color(1 - cl, cl);
        maxwidth = 256;
    }

    // Реализация рисования точки
    void pset(int x, int y, char cl) {

        DISPLAY(vm);

        if (x < 0 || y < 0 || x > 255 || y > 191) return;
        word z = ((x >> 3) | (y << 5));
        byte m = 1 << (7 ^ (x & 7));
        vm[z] = cl ? (vm[z] | m) : (vm[z] & ~m);
    }

    // Реализация рисования блока
    void block(int x1, int y1, int x2, int y2, byte cl) {

        DISPLAY(vm);

        // Ограничители
        if (x2 < 0 || y2 < 0 || x1 > 255 || y1 > 191) return;
        if (x1 < 0) x1 = 0; if (x2 > 255) x2 = 255;
        if (y1 < 0) y1 = 0; if (y2 > 191) y2 = 191;

        // Предвычисление
        byte x8a = (x1 >> 3);
        byte x8b = (x2 >> 3);
        word z   = (y1 << 5);
        word z2  = z + x8b;
        word za, zb;

        // Маски слева и справа
        word L = ((1 << (8 - (x1 & 7))) - 1);
        word R = (0xFF80 >>  (x2 & 7));

        // Если находится на одной линии 8 битной
        if (x8a == x8b) {

            z += x8a;
            byte m = L & R;
            for (int i = y1; i <= y2; i++) {
                if (cl) vm[z] |= m; else vm[z] &= ~m;
                z += 32;
            }
        }
        // Рисование слева и справа
        else {

            // Сохранение предыдущего значения
            zb = z;

            // Прорисовать вертикальные столбцы по 8 пикселей
            for (int x = x8a + (x1 & 7 ? 1 : 0); x < x8b; x++) {

                za = z + x;
                for (int y = y1; y <= y2; y++) { vm[za] = cl ? 255 : 0; za += 32; }
            }

            // Левый край
            z = zb + x8a;

            // Линии слева и справа
            for (int i = y1; i <= y2; i++) {

                if (cl) { vm[z] |=  L; vm[z2] |=  R; }
                else    { vm[z] &= ~L; vm[z2] &= ~R; }

                z  += 32;
                z2 += 32;
            }
        }
    }
};
