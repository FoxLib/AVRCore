#include <screen.cc>

screen D;       // Инициализация текстового видеорежима

int main() {

    D.cls(0x17);
    for(;;);
}
