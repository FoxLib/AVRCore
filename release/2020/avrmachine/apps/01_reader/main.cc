#include <avrio.cc>
#include <kb.cc>
#include <con.cc>

KB  kb;
CON d;

void draw_scrollbar() {

    // Ползунок
    for (int i = 1; i < 24; i++) d.prn(79, i, 0xB0);
    d.color(0x70); d.prn(79, 0, 0x18); d.prn(79, 24, 0x19);
    d.color(0x17); d.prn(79, 1, 0xB2);
}

int main() {

    d.init();
    d.cls(0x07);
    d.show(0);

    draw_scrollbar();
    for(;;);
}
