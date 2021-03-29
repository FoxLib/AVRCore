#include <gui.cc>

gui G;

int main() {

    G.init();
    G.cls(1);

    //G.window(100, 50, 320, 100);

    G.locate(78, 1);
    G.print("1234");

    for (;;);
}
