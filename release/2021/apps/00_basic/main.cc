#include <avrio.cc>

int main() {

    heap(vm, 0xf000);
    bank(2);

    for (int i = 0; i < 4000; i += 2) {
        vm[i] = i;
        vm[i+1] = 0x17;
    }

    for(;;);
}
