#include <screen13.cc>

int main() {

    Screen13 d(1);

    for (word y = 0; y < 200; y++)
    for (word x = 0; x < 320; x++) {

        int rn = 0, b = 512;
        while (b-- > 0) {
            int xn = x*b, yn = y*b;
            rn = (xn>>8) & (yn>>8) & b;

            if (rn & 0x40) { if (rn & 0x08) { d.pset(x, y, 44); } break; }
        }
    };

    for(;;);
}
