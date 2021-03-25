#include "graphics.cc"

enum WINDOW_PARAM {

    WIN_CLOSE    = 1,
    WIN_MAXIMIZE = 2,
    WIN_MINIMIZE = 4,
};

class gui : public graphics {

protected:

    // Каемка
    void btn(int x1, int y1, int x2, int y2, byte press) {

        block(x1, y1, x2, y2,  7);
        lineb(x1, y1, x2, y2,  press ? 15 : 8);
        block(x1, y1, x2, y1,  press ? 8  : 15);
        block(x1, y1, x1, y2,  press ? 8  : 15);
    }

public:

    // Нарисовать кнопку
    void button(int x, int y, int w, int h, byte press) {

        int x2 = x + w, y2 = y + h;

        block(x, y, x2, y2, 7);
        btn(x, y, x2, y2, press);
    }

    // Нарисовать окно
    void window(const char* title, int x, int y, int w, int h, int param) {

        int x2 = x + w, y2 = y + h;

        btn(x, y, x2, y2, 0);
        block(x+2, y+2, x2-2, y+12, 1);
        lineb(x-1, y-1, x2+1, y2+1, 0);
        locate(x+4, y+4); print(title);

        // Рисование крестика
        if (param & WIN_CLOSE) {

            button(x2 - 11, y + 3, 8, 8, 0);
            for (int i = 0; i < 5; i++) {
                pset(x2-9+i,y+i+5, 0);
                pset(x2-5-i,y+i+5, 0);
            }
        }
    }

    // Поле для ввода
    void textarea(int x, int y, int w, int h) {

        int x2 = x + w, y2 = y + h;
        block(x, y, x2, y2, 15);
        block(x, y, x2, y,  8);
        block(x, y, x,  y2, 8);
    }
};
