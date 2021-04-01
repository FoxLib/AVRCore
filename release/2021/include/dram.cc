#include "avrio.cc"

class dram {
protected:
public:

    dram() {
        while (inp(STATUS) & DRAM_BSY);
    }

    void address(dword address) {

        outp(DRAM3A, address >> 24);
        outp(DRAM2A, address >> 16);
        outp(DRAM1A, address >> 8);
        outp(DRAM0A, address);
    }

    // Запись байта в память
    void poke(dword _address, byte v) {
        address(_address);
        poke(v);
    }

    // Адрес уже установлен
    void poke(byte v) {

        outp(DRAMD,  v);
        outp(STATUS, inp(STATUS) | DRAM_WE);
        while (inp(STATUS) & DRAM_BSY);
        outp(STATUS, inp(STATUS) & ~DRAM_WE);
    }

    // Чтение одного байта из памяти
    byte peek(dword _address) {

        address(_address);
        return peek();
    }

    byte peek() {
        while (inp(STATUS) & DRAM_BSY);
        return inp(DRAMD);
    }
};
