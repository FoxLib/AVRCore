#include <avr/interrupt.h>
#include <screen.cc>
#include <sd.cc>

screen D;
SD     sd;

int main() {

    byte sector[512];

    D.init(7);
    D.print(" error: "); D.print( sd.get_status() );
    D.print(" type: ");  D.print( sd.get_sd_type() );
    D.print(" read: ");  D.print( sd.read(1, sector) );
    sector[0]++;
    D.print(" write:"); D.print( sd.write(1, sector) );
    D.print("\n");

    for (int i = 0; i < 512; i++) {

        if ((i & 31) == 0) { D.color(0x7); D.print(i,16,2); D.print(" "); }
        D.color(i & 1 ? 0x02 : 0x0A);
        D.print(sector[i], 16, 1);
        if ((i & 31) == 31) D.print("\n");
    }

    for (;;) { }
}
