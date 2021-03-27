class dram {
protected:
public:

    dram() {
        while (inp(STATUS) & DRAM_BSY);
    }

    // Запись байта в память
    void poke(dword address, byte v) {

        outp(DRAM0A, address);
        outp(DRAM1A, address >> 8);
        outp(DRAM2A, address >> 16);
        outp(DRAM3A, address >> 24);
        outp(DRAMD,  v);
        outp(STATUS, inp(STATUS) | DRAM_WE);
        while (inp(STATUS) & DRAM_BSY);
        outp(STATUS, inp(STATUS) & ~DRAM_WE);
    }

    // Чтение одного байта из памяти
    byte peek(dword address) {

        outp(DRAM0A, address);
        outp(DRAM1A, address >> 8);
        outp(DRAM2A, address >> 16);
        outp(DRAM3A, address >> 24);
        while (inp(STATUS) & DRAM_BSY);
        return inp(DRAMD);
    }

};
