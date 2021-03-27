#include <avr/interrupt.h>
#include <screen.cc>
#include <sd.cc>

screen D;
SD     S;

int main() {

    D.init(7);

    D.print( S.init() );

    for (;;) { }
}
