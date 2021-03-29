#include <screen3.cc>
#include <sd.cc>

screen3 D;
SD      sd;

/**
 * Тестирование чтения карты
 */

int main() {

    /*
    sd.read(1);
    sector[0]++; // Тест записи
    D.print(" write:"); D.print( sd.write(1) );
    D.print(" => ");    D.print( sd.get_error_code(), 16 );
    */

    D.init();
    D.cls();
    D.hide();

    for (int k = 0; k < 1; k++) {

        D.locate(0, 0);
        D.color(7);
        D.print(" error: "); D.print( sd.get_status() );
        D.print(" type: ");  D.print( sd.get_sd_type() );
        D.print(" id: ");    D.print(k);

        if ( sd.read(k) ) break; // , sector

        D.print("\n");
        for (int i = 0; i < 512; i++) {

            if ((i & 31) == 0) { D.color(0x7); D.print(i,16,2); D.print(" "); }
            D.color(i & 1 ? 0x02 : 0x0A);
            D.print( sd.sector[i], 16, 1);
            if ((i & 31) == 31) D.print("\n");
        }
    }

    for (;;) { }
}
