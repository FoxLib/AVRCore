#include <avrio.c>
#include <string.c>

#include <textmode.cpp>
#include <kb.cpp>
#include <dram.cpp>

/** Операционная система, основанная на исполнении программ BASIC
 */

KB       kb;
DRAM     dram;
TextMode t;

#include "core.cc"
TCore     core;

int main() {

    t.start();
    t.cls(0x07);

    core.main();
}
