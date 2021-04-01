#include <screen3.cc>
#include <dram.cc>

#define MAX 8*1024

screen3 D;
dram    M;

int main() {

    D.init();
    D.cls();
    D.locate(0, 0);

    // Запись значений
    for (dword k = 0; k < (dword)MAX*1024; k++) {

        if ((k & 0xff) == 0) M.address(k);
        outp(DRAM0A, k);
        M.poke(k);
        if ((k & 0xffff) == 0) D.print("#");
    }

    byte fail = 0;
    D.locate(0, 0);

    // Считывание и проверка
    for (dword k = 0; k < (dword)MAX*1024; k++) {

        byte m = M.peek(k);
        if (m != (k & 255)) fail = 1;
        if ((k & 0xffff) == 0) {

            D.color(fail ? 0x4f : 0x0a);
            D.print(fail ? "%" : "#");
            fail = 0;
        }
    }

    for (;;) {
    }
}
