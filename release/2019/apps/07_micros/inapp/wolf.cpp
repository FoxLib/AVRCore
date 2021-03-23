#include "wolf.h"

class AppWolf {
protected:

    byte  map[ WOLFDIM ][ WOLFDIM ];
    dword next;
    float px, py, alpha;
    float a_cos, a_sin;

public:

    void main() {

        float nx, ny;

        g.start();
        g.cls(1);
        mapgen();

        for (;;) {

            draw();

            byte ch = kb.getch();

            // Вперед
            if (ch == VK_UP) {
                nx = px + 0.5 * a_sin;
                ny = py + 0.5 * a_cos;
            }
            // Назад
            else if (ch == VK_DOWN)  {
                nx = px - 0.5 * a_sin;
                ny = py - 0.5 * a_cos;
            }
            // Повороты
            else if (ch == VK_LEFT)  { alpha -= 0.25; }
            else if (ch == VK_RIGHT) { alpha += 0.25; }
            else if (ch == VK_ESC) break;

            if (ch == VK_UP || ch == VK_DOWN) {
                if (map[ (int)ny ][ (int)nx ] == 0) {
                    px = nx;
                    py = ny;
                }
            }
        }
    }

    void draw() {

        heap(vm, 0xF000);

        word z;
        byte bounce;
        int tex_id;

        a_cos = sqrt2 * cos(alpha),
        a_sin = sqrt2 * sin(alpha);

        // Стартовая позиция точки
        int xi_ = (int) px; float x_ = px - xi_; float x; int xi;
        int yi_ = (int) py; float y_ = py - yi_; float y; int yi;

        // Просмотр горизонта событий
        for (int i = -160; i < 160; i += 2) {
    
            if (i == -80) viewmap();

            // Скопировать инициирующие точки
            x = x_; xi = xi_;
            y = y_; yi = yi_;

            // Вычисление смещения
            float fi = (float)i / 160.0;

            // Поворот точки
            float dx = a_cos*fi + a_sin;
            float dy = a_cos    - a_sin*fi;

            // Предвычисление
            float xa = (dx > 0) ? 1.0 :  0.0;
            float ya = (dy > 0) ? 1.0 :  0.0;
            float xb = (dx > 0) ? 1.0 : -1.0;
            float yb = (dy > 0) ? 1.0 : -1.0;
            float xc = 1.0 - xa;
            float yc = 1.0 - ya;
            float t  = 0.0, t1, t2;

            // Итерации по блокам
            for (int j = 0; j < 32; j++) {

                // Расчет разностей
                t1 = dx ? (xa - x) / dx : 100.0;
                t2 = dy ? (ya - y) / dy : 100.0;

                // Правая или левая стенка
                if (t1 < t2) {

                    x   = xc;
                    xi += xb;
                    y  += t1*dy;
                    t  += t1;
                    bounce = 0;

                    if      (y >= 1.0) y -= 1.0;
                    else if (y  < 0.0) y += 1.0;
                }
                // Верхняя или нижняя стена
                else {

                    y   = yc;
                    yi += yb;
                    x  += t2*dx;
                    t  += t2;
                    bounce = 1;

                    if      (x >= 1.0) x -= 1.0;
                    else if (x <  0.0) x += 1.0;
                }

                // Тест точки пересечения
                if (map[yi][xi]) {

                    // Вычисление Y = PPD / Z
                    t = 100.0 / t;
                    tex_id = map[yi][xi] - 1;

                    unsigned int cc;

                    // Текстура
                    cc = t < 30 ? 0x88 : (t < 50 ? 0x77 : 0xff);

                    // Границы
                    int y1 = 100 - t,
                        y2 = 100 + t;

                    // Смотря от чего отбился луч, оттуда взята текстура
                    int tx = (bounce ? x : y) * 32,
                        tym = 0,
                        ty  = 0,
                        dty = y2 - y1;

                    unsigned char sh1 = t > 50 ? 0xFF : 0xF0;
                    unsigned char sh2 = t > 50 ? 0x00 : (t > 30 ? 0x08 : 0x00);
                    //unsigned char shad = bounce ? 0xFF : 0xF0;

                    // Если начало стены находится за верхом
                    if (y1 < 0) {

                        tym  = 32 * (-y1);
                        ty  += (tym / dty);
                        tym %= dty;
                    }

                    // Сверху вниз заполнять линии
                    outp(0, 8); z = 80 + (i >> 1);

                    // Рисовать сверху вниз
                    for (int k = 0; k < 200; k++) {

                        // Расчет верха, стены и пола
                        if (k < y1) { cc = 0x11; }
                        // Рисование стены
                        else if (k < y2) {

                            // Целочисленное вычисление положения текстуры
                            tym += 32; while (tym > dty) { tym -= dty; ty++; }

                            // Вычисление картинки
                            cc = (wolf_texture[tex_id][ty & 15][tx & 15] & sh1) | sh2;

                            // Сеточка
                            sh1 = (sh1 >> 4) | (sh1 << 4);
                            sh2 = (sh2 >> 4) | (sh2 << 4);
                        }
                        // Рисовать пол
                        else cc = 0x22;

                        vm[z] = cc;
                        z    += 160;

                        // К следующему банку
                        if (z & 0x1000) { outp(0, inp(0) + 1); z &= 0xfff; }
                    }

                    break;
                }
            }
        }
    }

    void mapgen() {

        next = TIMERD;

        int i, j, first = 1;
        for (i = 0; i < WOLFDIM; i++)
        for (j = 0; j < WOLFDIM; j++) {

            byte gm = (i > 0 && j > 0 && i < WOLFDIM-1 && j < WOLFDIM-1) ? (random()&1 ? 1 : 0) : 1;
            map[i][j] = gm;

            if (gm == 0 && first) { px = j + 0.5; py = i + 0.5; first = 0; }
        }

        alpha = 0;

    }

    void viewmap() {

        for (int i = 0; i < WOLFDIM; i++)
        for (int j = 0; j < WOLFDIM; j++) {
            if (map[i][j]) {

                int xs = 8 + 4*j, ys = 8 + 4*i;
                for (int k = 0; k < 16; k++)
                    if (((k&3) ^ (k>>2)) & 1)
                        g.pset(xs + (k&3), ys + (k>>2), 10);
            }
        }
        g.pset(8 + 4*px, 8 + 4*py, 15);
    }

    byte random() {

        next = ((((next >> 31) ^ (next >> 30) ^ (next >> 29) ^ (next >> 27) ^ (next >> 25) ^ next ) & 1 ) << 31 ) | (next >> 1);
        return next;
    }
};
