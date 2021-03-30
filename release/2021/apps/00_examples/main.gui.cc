#include <gui.cc>

gui G;

int main() {

    G.init();
    G.cls(3);

    G.window(28, 62, 320, 100);

    G.locate(32, 64);
    G.print("Windows Not Must Die");

    for (;;);
}
