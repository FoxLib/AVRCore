#include <avr/interrupt.h>
#include <screen.cc>
#include <dram.cc>

screen D;
dram   M;

#define MAX 1*1024

int main() {

    D.init(7);
    D.locate(0, 0);

    // Запись значений
    for (dword k = 0; k < (dword)MAX*1024; k++) {
        M.poke(k, k);
        if ((k & 0xffff) == 0) D.print("#");
    }

    byte fail = 0;
    D.locate(0, 0);

    // Считывание и проверка
    for (dword k = 0; k < (dword)MAX*1024; k++) {
        byte m = M.peek(k);
        if (m != (k & 255)) fail = 1;
        if ((k & 0xffff) == 0) { D.print(fail ? "!" : "-"); fail = 0; }
    }

    for (;;) {
    }
}
