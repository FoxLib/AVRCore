#include <avrio.cc>
#include <kb.cc>
#include <con.cc>

KB  kb;
CON d;

int main() {

    d.init();
    d.cls(0x07);

    for(;;);
}
