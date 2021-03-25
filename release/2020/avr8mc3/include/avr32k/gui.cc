#include "graphics.cc"

struct RECT {
    int l, t, r, b;
};

class GUI : public graphics {
protected:

    RECT _bound[2];
    byte _bound_id;

public:

    // Установить экран как границу
    void bound() { bound(0, 0, 0, 255, 191); _bound_id = 0; }

    // Скопировать из ID -> 0
    void bound(int id) { bound(0, _bound[id].l, _bound[id].t, _bound[id].r, _bound[id].b); _bound_id = 0; }

    // Установить текущие границы для рисования элементов
    void bound(int id, int l, int t, int r, int b) {
        _bound[id].l = l;
        _bound[id].t = t;
        _bound[id].r = r;
        _bound[id].b = b;
        _bound_id = id;
    }

    // Рисуется окно
    int window(const char* title, int x, int y, int w, int h) {

        int r = x + w, b = y + h;

        block(x, y, r, b, 1);
        block(x+1, y+1, r-1, y+9, 0);
        locate(x+2, y+2);
        lineb(x-1, y-1, r+1, b+1, 0);
        print(title);

        block(r-8,y+2,r-2,y+8,1);
        for (int i = 0; i < 5; i++) {
            pset(r-7+i,y+3+i, 0);
            pset(r-3-i,y+3+i, 0);
        }

        // HWND=1 всегда
        bound(1, x+1, y+11, r-1, b-1);

        return 1;
    }

    RECT get_bound(int x, int y, int w, int h) {

        RECT r;
        r.l = _bound[_bound_id].l + x;
        r.t = _bound[_bound_id].t + y;
        r.r = r.l + w;
        r.b = r.t + h;
        return r;
    }

    RECT get_bound(int x, int y) { return get_bound(x, y, 0, 0); }

    // Текстовое поле
    void textarea(const char* title, int x, int y, int w) { textarea(title, x, y, w, 11); }
    void textarea(const char* title, int x, int y, int w, int h) {

        RECT r = get_bound(x, y, w, h);

        block(r.l, r.t, r.r, r.b, 1);
        lineb(r.l, r.t, r.r, r.b, 0);

        color(0, 1);
        locate(r.l + 2, r.t + 2);
        print(title);
    }

    // Кнопка
    void button(int x, int y, int w, int h, int press) {

        RECT r = get_bound(x, y, w, h);

        if (press) {

            block(r.l, r.t, r.r, r.t, 0);
            block(r.l, r.t, r.l, r.b, 0);

        } else {

            block(r.r, r.t, r.r, r.b, 0);
            block(r.l, r.b, r.r, r.b, 0);
        }
    }

    // Нарисовать кнопку с текстом
    void button(const char* title, int x, int y) {

        RECT r = get_bound(x, y);

        locate(r.l + 3, r.t + 2);
        int m = print(title);

        button(x, y, m*4 + 4, 11, 0);
    }
};
