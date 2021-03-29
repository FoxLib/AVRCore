#include <screen12.cc>

screen12 G;

int main() {

    G.init();
    G.cls(0);

    for (int i = 0; i < 200; i++)
        G.block(i,i,639-i,399-i,i&1?7:8);

    for (;;);
}
