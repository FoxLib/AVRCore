#include <screen13.cc>

screen13 G;

int main() {

    G.cls();

    // Координаты треугольника
    int Ax = -5, Ay =  5, Az = 6;
    int Bx =  5, By =  5, Bz = 6;
    int Cx = -5, Cy = -5, Cz = 6;

    // Предварительный расчет
    int ABx = Bx - Ax, ACx = Cx - Ax;
    int ABy = By - Ay, ACy = Cy - Ay;
    int ABz = Bz - Az, ACz = Cz - Az;

    int A1 = ( Ay * ACz -  Az * ACy), A2 = ( Az * ACx -  Ax * ACz), A3 = ( Ax * ACy -  Ay * ACx);
    int B1 = ( Az * ABy -  Ay * ABz), B2 = ( Ax * ABz -  Az * ABx), B3 = ( Ay * ABx -  Ax * ABy);
    int C1 = (ABz * ACy - ABy * ACz), C2 = (ABx * ACz - ABz * ACx), C3 = (ABy * ACx - ABx * ACy);

    for (int y = 0; y < 200; y++) {

        // Проекционный луч
        int dx = 0-160;
        int dy = 100 - y;
        int dz = 100;

        // Расчет координат
        long u = dx*A1 + dy*A2 + dz*A3;
        long v = dx*B1 + dy*B2 + dz*B3;
        long D = dx*C1 + dy*C2 + dz*C3;

        for (int x = 0; x < 320; x++) {

            // Плоскость не должна быть параллельна лучу проекции
            if (D != 0) {

                if (u >= 0 && v >= 0 && u + v <= D) {

                    int u_ = (255*u) / D;
                    int v_ = (255*v) / D;
                    int c = u_ ^ v_;

                    G.setpixel(x, y, 0, c, 0);
                    //G.pset(x, y, c);
                }
            }

            dx++;
            u += A1;
            v += B1;
            D += C1;
        }
    }

    for (;;);
}
