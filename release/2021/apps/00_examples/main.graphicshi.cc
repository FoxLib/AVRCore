#include <graphicshi.cc>

graphicshi G;

int main() {

    G.init();
    G.cls(0);

    for (int i = 1; i < 540; i++)
        G.block(i,1,i+100,100,i&1?7:8);

    for (;;);
}
