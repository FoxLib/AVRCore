
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

enum RegsALL {

    REG_AX  = 0,    REG_AL = 0,
    REG_CX  = 1,    REG_CL = 1,
    REG_DX  = 2,    REG_DL = 2,
    REG_BX  = 3,    REG_BL = 3,
    REG_SP  = 4,    REG_AH = 4,
    REG_BP  = 5,    REG_CH = 5,
    REG_SI  = 6,    REG_DH = 6,
    REG_DI  = 7,    REG_BH = 7,
};

enum SegID {

    SEG_ES = 0,
    SEG_CS = 1,
    SEG_SS = 2,
    SEG_DS = 3
};

enum Prefixes {

    REPNZ = 1,
    REPZ  = 2

};

uint8_t  opcode, rmdat, cpu_reg, cpu_mod, cpu_rm;
uint8_t  sel_seg, rep;
uint16_t eaaddr;
uint32_t segment;

uint8_t  znptable8[256];    // Специальная таблица с Z,S,P
uint16_t regs[8];
uint16_t segs[4];
uint32_t seg_cs, seg_ss, seg_es, seg_ds;
uint16_t ip;
