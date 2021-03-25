#include <stdio.cc>
#include <avr32k/gui.cc>

GUI D;
stdio io;

int main() {

    D.cls(0);

    for (int i = 0; i < 256; i++) D.pset(io.rand()&255, io.rand()%192, 1);

    D.circlef(255, 120, 128, 1);
    D.locate(8, 8);
    D.print("Hello world! This 256x192 pix, 6kb. 32kb FLASH ROM. 8kb RAM");

    D.window("Windows XP: Corporational Edition", 10, 30, 150, 50);
    D.textarea("2+2*2=+100500", 0, 0, 140);
    D.button("OK", 0, 15);


    D.block(0, 178, 319, 191, 1);
    D.block(0, 178, 319, 178, 0);
    D.line (2, 190, 24, 190, 0);
    D.line (24, 180, 24, 190, 0);
    D.locate(4, 182); D.print("START");

    // Бесконечный цикл
    for(;;);
}
