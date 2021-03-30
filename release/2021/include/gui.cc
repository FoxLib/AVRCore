#include "screen12.cc"

class gui: public screen12 {
protected:
public:

    void window(int x1, int y1, int w, int h) {

        int x2 = x1 + w, y2 = y1 + h;
        block(x1, y1, x2, y2, 7);
        line(x2, y1, x2, y2, 8);
        line(x1, y2, x2, y2, 8);
        line(x1, y1, x2, y1, 15);
        line(x1, y1, x1, y2, 15);
        block(x1+2,y1+2,x2-2,y1+18,1);
    }
};
