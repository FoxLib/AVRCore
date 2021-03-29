#include <gui.cc>

gui G;

int main() {

    G.init();
    G.cls(0);

    G.window(100, 50, 320, 100);

    for (;;);
}
