#include <avr8k/gui.cc>

gui D;

// Шаблон с чтением из PGM
int main() {

    D.cls(3);

    D.window("CALCULATOR", 8, 8, 100, 128, WIN_CLOSE);

    D.textarea(10, 22, 96, 12);
    D.locate(13, 25); D.color(0); D.print("2+2=4");

    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 3; j++) {

        int x = 13 + 32*j,
            y = 40 + 24*i;

        D.button(x, y, 24, 16, 0);
        D.locate(x + 11, y + 5);

        int n = i*3 + j + 1;
        if (n <= 9) D.print(n);
        else if (n == 11) D.print((long)0);
    }


    // Бесконечный цикл
    for(;;);
}
