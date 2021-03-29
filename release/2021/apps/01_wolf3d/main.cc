#include <math.h>

#include <screen13.cc>
#include <keyboard.cc>
#include "resources.h"

screen13 G;
keyboard K;

int main() {

    float px = 1.5,
          py = 1.5,
          a = 0.0;

    byte bounce;
    int  tex_id;

    byte cl11  = G.rgb(  0,  0,    0);
    byte cl22  = G.rgb( 32, 128,  32);
    byte cl22h = G.rgb( 16,  64,  16);

    heap(vm, 0xf000);

    int  buffer_id = 0;
    byte kpad = 0;

    G.screen(0);

    for (;;) {

        int current_bank = buffer_id ? 0x20 : 0x30;

        // Предвычисление
        float a_cos = sqrt2 * cos(a),
              a_sin = sqrt2 * sin(a);

        // Стартовая позиция точки
        int xi_ = (int) px; float x_ = px - xi_; float x; int xi;
        int yi_ = (int) py; float y_ = py - yi_; float y; int yi;

        // Просмотр горизонта событий
        for (int i = -160; i < 160; i += 2) {

            // Скопировать инициирующие точки
            x = x_; xi = xi_;
            y = y_; yi = yi_;

            // Вычисление смещения
            float fi = -(float)i / 160.0;

            // Поворот точки
            float dx = a_cos*fi - a_sin;
            float dy = a_cos    + a_sin*fi;

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

                    unsigned int cc;

                    bank(current_bank);
                    word z = i + 160;

                    // Вычисление Y = PPD / Z
                    t      = 100.0 / t;
                    tex_id = map[yi][xi] - 1;

                    // Границы
                    int y1 = 100 - t,
                        y2 = 100 + t;

                    // Смотря от чего отбился луч, оттуда взята текстура
                    int tx = (bounce ? x : y) * 32,
                        tym = 0,
                        ty  = 0,
                        dty = y2 - y1;

                    // Если начало стены находится за верхом
                    if (y1 < 0) {

                        tym  = 32 * (-y1);
                        ty  += (tym / dty);
                        tym %= dty;
                    }

                    // Рисовать сверху вниз
                    for (int k = 0; k < 200; k++) {

                        // Расчет верха, стены и пола
                        if (k < y1) cc = cl11;
                        // Рисование стены
                        else if (k < y2) {

                            // Целочисленное вычисление положения текстуры
                            tym += 32; while (tym > dty) { tym -= dty; ty++; }
                            cc = tex[tex_id][ty&15][tx&15];

                            // Затенение
                            if (t < 70) {

                                if (t > 35)
                                     cc = (cc >> 1) & 0b01101101;
                                else cc = (cc >> 2) & 0b00100100;
                            }
                        }
                        // Рисовать пол с затенением
                        else {

                            if (k < 135) cc = 0;
                            else if (k < 170) cc = cl22h;
                            else cc = cl22;
                        }

                        // Более скоростное рисование
                        vm[z]   = cc;
                        vm[z+1] = cc;

                        z += 320; if (z >= 4096) { bank(inp(0) + 1); z -= 4096; }
                    }

                    break;
                }
            }

            // Регистрация событий клавиатуры
            switch (K.get()) {

                case 'w': kpad |= 1; break;
                case 's': kpad |= 2; break;
                case 'a': kpad |= 4; break;
                case 'd': kpad |= 8; break;
                case key_LF: kpad |= 16; break;
                case key_RT: kpad |= 32; break;
                case 'w' | 0x80: kpad &= ~1; break;
                case 's' | 0x80: kpad &= ~2; break;
                case 'a' | 0x80: kpad &= ~4; break;
                case 'd' | 0x80: kpad &= ~8; break;
                case key_LF | 0x80: kpad &= ~16; break;
                case key_RT | 0x80: kpad &= ~32; break;
            }
        }

        // Обмен банков местами
        buffer_id = 1 - buffer_id;
        G.screen(buffer_id);

        float _px = px, _py = py;

        if      (kpad & 1) { px -= 0.25 * a_sin; py += 0.25 * a_cos; }
        else if (kpad & 2) { px += 0.25 * a_sin; py -= 0.25 * a_cos; }

        if      (kpad & 4) { px += 0.25 * a_cos; py += 0.25 * a_sin; }
        else if (kpad & 8) { px -= 0.25 * a_cos; py -= 0.25 * a_sin; }

        if      (kpad & 16) { a -= 0.25; }
        else if (kpad & 32) { a += 0.25; }

        // Запретить проходить через стены
        if (map[(int)py][(int)px]) { px = _px; py = _py; }
    }
}
