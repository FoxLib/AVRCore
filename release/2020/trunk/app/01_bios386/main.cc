#include <screen3.cc>

#include "print.cc"

int main() {

    cls(0x07);

    // Суперски стартовый экраноид
    locate(0, 1);   print("AMIBIOS (C)1993 American Megatrends Inc.");
    locate(0, 6);   print("Hit <DEL>, If you want to run SETUP");
    locate(0, 23);  print("(C) American Megatrends Inc.");
    locate(0, 24);  print("40-020F-001133-00101111-040493-ALI1429-0");

    for(;;);
}
