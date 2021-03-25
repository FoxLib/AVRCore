#include <screen.cc>

screen D; // Инициализация текстового видеорежима

int main() {

    D.cls(0x17);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");
    D.print(-777333);

    for(;;) {

        int k = inp(0x01);
        D.locate(2, 2);
        D.print( k );
        D.print("  ");
    }
}
