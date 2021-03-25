#include <screen.cc>

screen D; // Инициализация текстового видеорежима

int main() {

    D.cls(0x17);
    D.locate(2, 1);
    D.print("What's love? It's a happy! ");
    D.print(-777333);

    byte pk = 0;

    for(;;) {

        while (((inp(0x02)^pk)&1) == 0); pk = inp(0x02);

        int k = inp(0x01);

        D.print(k);
        D.print(" ");
    }
}
