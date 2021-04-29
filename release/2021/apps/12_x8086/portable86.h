
uint16_t flags;

enum FlagBit
{
    C_FLAG = 0x0001,
    P_FLAG = 0x0004,
    A_FLAG = 0x0010,
    Z_FLAG = 0x0040,
    N_FLAG = 0x0080,
    T_FLAG = 0x0100,
    I_FLAG = 0x0200,
    D_FLAG = 0x0400,
    V_FLAG = 0x0800
};

enum SegID {

    seg_ES = 0,
    seg_CS = 1,
    seg_SS = 2,
    seg_DS = 3
};

uint8_t  znptable8[256];    // Специальная таблица с Z,S,P
uint16_t regs[8];
uint16_t segs[4];
uint16_t ip;
