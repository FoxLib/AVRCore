#include <screen13.cc>
#include <math.h>

screen13 G;

int main() {

    int vect[8][2];

    float ppd = 100;
    float cam[3] = {0, -1.5, 2.5};

    float vertex[8][3] = {
        {-1,1,1},  {1,1,1},  {1,-1,1},  {-1,-1,1},
        {-1,1,-1}, {1,1,-1}, {1,-1,-1}, {-1,-1,-1}
    };

    int faces[6][4] = {
        {1,0,3,2}, {4,5,6,7}, {0,1,5,4},
        {2,3,7,6}, {5,1,2,6}, {0,4,7,3}
    };

    float roty = 0.1;

    G.init();
    G.flip();

    for (;;) {

        G.cls(0);

        float cosa = cos(roty),
              sina = sin(roty);

        // Предкомпиляция точек
        for (int i = 0; i < 8; i++) {

            float x = cosa*vertex[i][0] - sina*vertex[i][2];
            float y =      vertex[i][1];
            float z = cosa*vertex[i][2] + sina*vertex[i][0];

            vect[i][0] = 160 + ppd*(cam[0]+x) / (z+cam[2]);
            vect[i][1] = 100 - ppd*(cam[1]+y) / (z+cam[2]);
        }

        for (int i = 0; i < 6; i++) {

            int a = faces[i][0], b = faces[i][1], c = faces[i][2];

            int x   = vect[ a ][0],   y   = vect[ a ][1];
            int ABx = vect[ b ][0]-x, ABy = vect[ b ][1]-y,
                ACx = vect[ c ][0]-x, ACy = vect[ c ][1]-y;

            if (ACx*ABy <= ACy*ABx)
            for (int j = 0; j < 4; j++) {

                int a = faces[i][j],
                    b = faces[i][(j+1)&3];

                G.line(vect[a][0], vect[a][1], vect[b][0], vect[b][1], 255);
            }
        }

        G.flip();

        roty += 0.01;
    }
}
