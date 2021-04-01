#include <screen3.cc>

int main() {

    screen3 D; D.init(); D.cls(7);

    float t = 1.1;
    while (t < 10) {

        D.print(t*t);
        D.print("; ");

        t *= 1.01;
    }

    for (;;);
}
