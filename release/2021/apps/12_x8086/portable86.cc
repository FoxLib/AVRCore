#include "portable86.h"

// ---------------------------------------------------------------------
// ПРОЦЕДУРЫ-ХЕЛПЕРЫ ДЛЯ АЛУ
// ---------------------------------------------------------------------

// Базовые флаги N,Z,P после 8-битных инструкции
void setznp8(uint8_t val)
{
    flags &= ~0xC4;
    flags |= znptable8[val];
}

// Базовые флаги N,Z,P после 16-битных инструкции
void setznp16(uint16_t val)
{
    flags &= ~0xC4;

    if (val & 0x8000) flags |= N_FLAG;
    if (val == 0)     flags |= Z_FLAG;
    flags |= (znptable8[val&0xff] & P_FLAG);
}

// Изменение флагов после ADD-8
uint8_t setadd8(uint8_t a, uint8_t b)
{
    uint16_t c = (uint16_t)a + (uint16_t)b;

    flags &= ~0x8D5;
    flags |= znptable8[c & 0xFF];

    if (c & 0x100) flags |= C_FLAG;
    if (!((a^b)&0x80)&&((a^c)&0x80)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)      flags |= A_FLAG;

    return c;
}

// Изменение флагов после ADD-8 без переноса CF
uint8_t setadd8nc(uint8_t a, uint8_t b)
{
    uint16_t c = (uint16_t)a + (uint16_t)b;

    flags &= ~0x8D4;
    flags |= znptable8[c&0xFF];

    if (!((a^b)&0x80)&&((a^c)&0x80)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)      flags |= A_FLAG;

    return c;
}

uint8_t setadc8(uint8_t a, uint8_t b, uint8_t tempc)
{
    uint16_t c = (uint16_t)a + (uint16_t)b + tempc;

    flags &= ~0x8D5;
    flags |= znptable8[c&0xFF];

    if (c & 0x100) flags |= C_FLAG;
    if (!((a^b)&0x80)&&((a^c)&0x80)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)       flags |= A_FLAG;

    return c;
}

uint8_t setadd16(uint16_t a, uint16_t b)
{
    uint32_t c = (uint32_t)a + (uint32_t)b;

    flags &= ~0x8D5;
    setznp16(c & 0xffff);

    if (c & 0x10000) flags |= C_FLAG;
    if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)          flags |= A_FLAG;

    return c;
}

uint16_t setadd16nc(uint16_t a, uint16_t b)
{
    uint32_t c = (uint32_t)a + (uint32_t)b;

    flags &= ~0x8D4;
    setznp16(c & 0xffff);

    if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)          flags |= A_FLAG;

    return c;
}

uint16_t setadc16(uint16_t a, uint16_t b, uint8_t tempc)
{
    uint32_t c = (uint32_t)a + (uint32_t)b + tempc;

    flags &= ~0x8D5;
    setznp16(c & 0xffff);

    if (c & 0x10000) flags |= C_FLAG;
    if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags |= V_FLAG;
    if (((a&0xF)+(b&0xF))&0x10)          flags |= A_FLAG;

    return c;
}

uint8_t setsub8(uint8_t a, uint8_t b)
{
    uint16_t c = (uint16_t)a - (uint16_t)b;

    flags &= ~0x8D5;
    flags |= znptable8[c&0xFF];

    if (c&0x100) flags|=C_FLAG;
    if ((a^b)&(a^c)&0x80)       flags|=V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags|=A_FLAG;

    return c;
}

uint8_t setsub8nc(uint8_t a, uint8_t b)
{
    uint16_t c=(uint16_t)a-(uint16_t)b;

    flags &= ~0x8D4;
    flags |= znptable8[c&0xFF];

    if ((a^b)&(a^c)&0x80)       flags |= V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags |= A_FLAG;

    return c;
}

uint8_t setsbc8(uint8_t a, uint8_t b, uint8_t tempc)
{
    uint16_t c = (uint16_t)a - (((uint16_t)b) + tempc);

    flags &= ~0x8D5;
    flags |= znptable8[c&0xFF];

    if (c&0x100)                flags |= C_FLAG;
    if ((a^b)&(a^c)&0x80)       flags |= V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags |= A_FLAG;

    return c;
}

uint16_t setsub16(uint16_t a, uint16_t b)
{
    uint32_t c = (uint32_t)a - (uint32_t)b;

    flags &= ~0x8D5;
    setznp16(c & 0xffff);

    if (c&0x10000) flags |= C_FLAG;
    if ((a^b)&(a^c)&0x8000)     flags |= V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags |= A_FLAG;

    return c;
}

uint16_t setsub16nc(uint16_t a, uint16_t b)
{
    uint32_t c=(uint32_t)a-(uint32_t)b;

    flags &= ~0x8D4;
    setznp16(c & 0xffff);
    flags &= ~0x4;
    flags |= (znptable8[c&0xFF]&4);

    if ((a^b)&(a^c)&0x8000)     flags |= V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags |= A_FLAG;

    return c;
}

uint16_t setsbc16(uint16_t a, uint16_t b, uint8_t tempc)
{
    uint32_t c = (uint32_t)a - (((uint32_t)b) + tempc);

    flags &= ~0x8D5;
    setznp16(c & 0xffff);
    flags &= ~0x4;
    flags |= (znptable8[c&0xFF]&4);

    if (c&0x10000)              flags |= C_FLAG;
    if ((a^b)&(a^c)&0x8000)     flags |= V_FLAG;
    if (((a&0xF)-(b&0xF))&0x10) flags |= A_FLAG;

    return c;
}

uint8_t setor8(uint8_t a, uint8_t b) {

    uint8_t c = a | b;

    setznp8(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

uint16_t setor16(uint16_t a, uint16_t b) {

    uint16_t c = a | b;

    setznp16(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

uint8_t setand8(uint8_t a, uint8_t b) {

    uint8_t c = a & b;

    setznp8(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

uint16_t setand16(uint16_t a, uint16_t b) {

    uint16_t c = a & b;

    setznp16(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

uint8_t setxor8(uint8_t a, uint8_t b) {

    uint8_t c = a ^ b;

    setznp8(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

uint16_t setxor16(uint16_t a, uint16_t b) {

    uint16_t c = a ^ b;

    setznp16(c);
    flags &=~ (C_FLAG | V_FLAG | A_FLAG);

    return c;
}

// ---------------------------------------------------------------------
// СЧИТЫВАНИЕ И ДЕКОДИРОВАНИЕ
// ---------------------------------------------------------------------

// Читать слово из памяти
uint16_t readmemw(uint32_t s, uint16_t a) {

    uint8_t l = readmemb(s + a);
    uint8_t h = readmemb(s + ((a+1) & 0xffff));
    return (h<<8) | l;
}

// Писать слово в память
void writememw(uint32_t s, uint16_t a, uint16_t v) {

    writememb(s + a, v);
    writememb(s + ((a+1) & 0xffff), v >> 8);
}

// Загрузка нового значения CS
void loadcs(uint16_t s) { segs[SEG_CS] = s; seg_cs = (uint32_t)s << 4; }
void loadss(uint16_t s) { segs[SEG_SS] = s; seg_ss = (uint32_t)s << 4; }
void loades(uint16_t s) { segs[SEG_ES] = s; seg_es = (uint32_t)s << 4; }
void loadds(uint16_t s) { segs[SEG_DS] = s; seg_ds = (uint32_t)s << 4; }

// Считывание следующего кода из CS:IP
uint8_t FETCH() { return readmemb(seg_cs + (ip++)); }

// Считывание WORD из потока CS:IP
uint16_t getword() { uint8_t l = FETCH(); return ((uint16_t)FETCH() << 8) | l; }

// Прочитать эффективный адрес
void fetchea() {

    rmdat   = FETCH();
    cpu_reg = (rmdat >> 3) & 7;
    cpu_mod = (rmdat >> 6) & 3;
    cpu_rm  = rmdat & 7;
    eaaddr  = 0;

    // Расчет эффективного адреса
    switch (cpu_rm) {

        case 0: eaaddr = (regs[REG_BX] + regs[REG_SI]); break;
        case 1: eaaddr = (regs[REG_BX] + regs[REG_DI]); break;
        case 2: eaaddr = (regs[REG_BP] + regs[REG_SI]); break;
        case 3: eaaddr = (regs[REG_BP] + regs[REG_DI]); break;
        case 4: eaaddr =  regs[REG_SI]; break;
        case 5: eaaddr =  regs[REG_DI]; break;
        case 6: eaaddr =  regs[REG_BP]; break;
        case 7: eaaddr =  regs[REG_BX]; break;
    }

    // Выбор сегмента
    if (sel_seg == 0 && ((cpu_rm == 6 && cpu_mod) || (cpu_rm == 2) || (cpu_rm == 3)))
        segment = seg_ss;

    // Модифицирующие биты modrm
    switch (cpu_mod) {

        case 0: if (cpu_rm == 6) eaaddr = getword(); break;
        case 1: eaaddr += (signed char) FETCH(); break;
        case 2: eaaddr += getword(); break;
        case 3: eaaddr = 0; break;
    }
}

// Чтение из 8-битного регистра
uint8_t getr8(uint8_t id) { return id & 4 ? regs[id&3] >> 8 : regs[id&3]; }

// Запись в 8-битный регистр
void setr8(uint8_t id, uint8_t v) {

    if (id & 4) {
        regs[id&3] = (regs[id&3] & 0x00ff) | ((uint16_t)v << 8);
    } else {
        regs[id&3] = (regs[id&3] & 0xff00) | v;
    }
}

// Чтение и запись в 16-битные регистры
inline uint16_t getr16(uint8_t id) { return regs[id]; }
inline void     setr16(uint8_t id, uint16_t v) { regs[id] = v; }

// Прочитать byte из r/m
uint8_t geteab() {

    if (cpu_mod == 3)
        return getr8(cpu_rm);
    else
        return readmemb(segment + eaaddr);
}

// Прочитать word из r/m
uint16_t geteaw() {

    if (cpu_mod == 3) return getr16(cpu_rm);
    return readmemw(segment, eaaddr);
}

// Записать byte в r/m
void seteab(uint8_t v) {

    if (cpu_mod == 3) {
        setr8(cpu_rm, v);
    } else {
        writememb(segment + eaaddr, v);
    }
}

// Записать word в r/m
void seteaw(uint16_t v) {

    if (cpu_mod == 3) {
        regs[cpu_rm] = v;
    } else {
        writememw(segment, eaaddr, v);
    }
}

// Запись в стек
void push(uint16_t v) {

    writememw(seg_ss, ((regs[REG_SP] - 2) & 0xffff), v);
    regs[REG_SP] -= 2;
}

// Извлечение из стека
uint16_t pop() {

    uint16_t r = readmemw(seg_ss, regs[REG_SP]);
    regs[REG_SP] += 2;
    return r;
}

// Десятичная коррекция после сложения
void daa() {

    uint8_t  AL = regs[REG_AL];
    uint16_t tempi;

    if ((flags & A_FLAG) || ((AL & 0xF) > 9))
    {
        tempi = ((uint16_t) AL) + 6;
        AL += 6;
        flags |= A_FLAG;
        if (tempi & 0x100) flags |= C_FLAG;
    }

    if ((flags & C_FLAG) || (AL > 0x9F))
    {
        AL += 0x60;
        flags |= C_FLAG;
    }

    setznp8(AL);
    setr8(REG_AL, AL);
}

// Десятичная коррекция после вычитания
void das() {

    uint8_t  AL = regs[REG_AL];
    uint16_t tempi;

    if ((flags & A_FLAG) || ((AL & 0xF)>9))
    {
        tempi = ((uint16_t)AL) - 6;
        AL -= 6;
        flags |= A_FLAG;
        if (tempi & 0x100) flags |= C_FLAG;
    }

    if ((flags & C_FLAG) || (AL>0x9F))
    {
        AL -= 0x60;
        flags |= C_FLAG;
    }

    setznp8(AL);
    setr8(REG_AL, AL);
}

// ASCII-коррекция после сложения
void aaa() {

    uint8_t  AL = regs[REG_AL];
    uint8_t  AH = regs[REG_AX] >> 8;

    if ((flags & A_FLAG) || ((AL & 0xF) > 9))
    {
        AL+=6;
        AH++;
        flags |= (A_FLAG|C_FLAG);
    }
    else
       flags &= ~(A_FLAG|C_FLAG);

    setr16(REG_AX, (AL&15) + ((uint16_t)AH << 8));
}

// ASCII-коррекция после вычитания
void aas() {

    uint8_t  AL = regs[REG_AL];
    uint8_t  AH = regs[REG_AX] >> 8;

    if ((flags & A_FLAG) || ((AL & 0xF) > 9))
    {
        AL-=6;
        AH--;
        flags |= (A_FLAG|C_FLAG);
    }
    else
       flags &= ~(A_FLAG|C_FLAG);

    setr16(REG_AX, (AL&15) + ((uint16_t)AH<<8));
}

// ---------------------------------------------------------------------
// ЦИКЛ ВЫПОЛНЕНИЯ ОДНОЙ ИНСТРУКЦИИ
// ---------------------------------------------------------------------

// Заполнение данными
void initcpu() {

    byte d, c = 0;

    do
    {
        d = (c>>4) ^ c;
        d = (d>>2) ^ d;
        d = (d>>1) ^ d;

        znptable8[c] = (d&1) ? 0 : P_FLAG;
        if (c == 0) znptable8[c] |= Z_FLAG;
        if (c&0x80) znptable8[c] |= N_FLAG;

        c++;
    }
    while (c != 0);

    loadcs(0xf000);
    ip = 0x0000;
}

// Одна инструкция
void step() {

    uint8_t  cont = 0;
    uint8_t  tempc = flags & C_FLAG;

    rep     = 0;
    sel_seg = 0;
    segment = seg_ds;

    do {

        opcode = FETCH();

        switch (opcode) {

            // ADD
            case 0x00: fetchea(); seteab(setadd8 (geteab(), getr8 (cpu_reg))); break;
            case 0x01: fetchea(); seteab(setadd16(geteaw(), getr16(cpu_reg))); break;
            case 0x02: fetchea(); setr8 (cpu_reg, setadd8 (getr8 (cpu_reg), geteab())); break;
            case 0x03: fetchea(); setr16(cpu_reg, setadd16(getr16(cpu_reg), geteaw())); break;
            case 0x04: setr8 (REG_AL, setadd8 (regs[REG_AL], FETCH())); break;
            case 0x05: setr16(REG_AX, setadd16(regs[REG_AX], FETCH())); break;
            case 0x06: push(segs[SEG_ES]); break;
            case 0x07: loades(pop()); break;

            // OR
            case 0x08: fetchea(); seteab(setor8 (geteab(), getr8 (cpu_reg))); break;
            case 0x09: fetchea(); seteab(setor16(geteaw(), getr16(cpu_reg))); break;
            case 0x0A: fetchea(); setr8 (cpu_reg, setor8 (getr8 (cpu_reg), geteab())); break;
            case 0x0B: fetchea(); setr16(cpu_reg, setor16(getr16(cpu_reg), geteaw())); break;
            case 0x0C: setr8 (REG_AL, setor8 (regs[REG_AL], FETCH())); break;
            case 0x0D: setr16(REG_AX, setor16(regs[REG_AX], FETCH())); break;
            case 0x0E: push(segs[SEG_CS]); break;
            case 0x0F: /* EXTENDED */ FETCH(); break;

            // ADС
            case 0x10: fetchea(); seteab(setadc8 (geteab(), getr8 (cpu_reg), tempc)); break;
            case 0x11: fetchea(); seteab(setadc16(geteaw(), getr16(cpu_reg), tempc)); break;
            case 0x12: fetchea(); setr8 (cpu_reg, setadc8 (getr8 (cpu_reg), geteab(), tempc)); break;
            case 0x13: fetchea(); setr16(cpu_reg, setadc16(getr16(cpu_reg), geteaw(), tempc)); break;
            case 0x14: setr8 (REG_AL, setadc8 (regs[REG_AL], FETCH(), tempc)); break;
            case 0x15: setr16(REG_AX, setadc16(regs[REG_AX], FETCH(), tempc)); break;
            case 0x16: push(segs[SEG_SS]); break;
            case 0x17: loadss(pop()); break;

            // SBB
            case 0x18: fetchea(); seteab(setsbc8 (geteab(), getr8 (cpu_reg), tempc)); break;
            case 0x19: fetchea(); seteab(setsbc16(geteaw(), getr16(cpu_reg), tempc)); break;
            case 0x1A: fetchea(); setr8 (cpu_reg, setsbc8 (getr8 (cpu_reg), geteab(), tempc)); break;
            case 0x1B: fetchea(); setr16(cpu_reg, setsbc16(getr16(cpu_reg), geteaw(), tempc)); break;
            case 0x1C: setr8 (REG_AL, setsbc8 (regs[REG_AL], FETCH(), tempc)); break;
            case 0x1D: setr16(REG_AX, setsbc16(regs[REG_AX], FETCH(), tempc)); break;
            case 0x1E: push(segs[SEG_DS]); break;
            case 0x1F: loadds(pop()); break;

            // AND
            case 0x20: fetchea(); seteab(setand8 (geteab(), getr8 (cpu_reg))); break;
            case 0x21: fetchea(); seteab(setand16(geteaw(), getr16(cpu_reg))); break;
            case 0x22: fetchea(); setr8 (cpu_reg, setand8 (getr8 (cpu_reg), geteab())); break;
            case 0x23: fetchea(); setr16(cpu_reg, setand16(getr16(cpu_reg), geteaw())); break;
            case 0x24: setr8 (REG_AL, setand8 (regs[REG_AL], FETCH())); break;
            case 0x25: setr16(REG_AX, setand16(regs[REG_AX], FETCH())); break;
            case 0x26: sel_seg = 1; segment = seg_es; cont = 1; break;
            case 0x27: daa(); break;

            // SUB
            case 0x28: fetchea(); seteab(setsub8 (geteab(), getr8 (cpu_reg))); break;
            case 0x29: fetchea(); seteab(setsub16(geteaw(), getr16(cpu_reg))); break;
            case 0x2A: fetchea(); setr8 (cpu_reg, setsub8 (getr8 (cpu_reg), geteab())); break;
            case 0x2B: fetchea(); setr16(cpu_reg, setsub16(getr16(cpu_reg), geteaw())); break;
            case 0x2C: setr8 (REG_AL, setsub8 (regs[REG_AL], FETCH())); break;
            case 0x2D: setr16(REG_AX, setsub16(regs[REG_AX], FETCH())); break;
            case 0x2E: sel_seg = 1; segment = seg_cs; cont = 1; break;
            case 0x2F: das(); break;

            // XOR
            case 0x30: fetchea(); seteab(setxor8 (geteab(), getr8 (cpu_reg))); break;
            case 0x31: fetchea(); seteab(setxor16(geteaw(), getr16(cpu_reg))); break;
            case 0x32: fetchea(); setr8 (cpu_reg, setxor8 (getr8 (cpu_reg), geteab())); break;
            case 0x33: fetchea(); setr16(cpu_reg, setxor16(getr16(cpu_reg), geteaw())); break;
            case 0x34: setr8 (REG_AL, setxor8 (regs[REG_AL], FETCH())); break;
            case 0x35: setr16(REG_AX, setxor16(regs[REG_AX], FETCH())); break;
            case 0x36: sel_seg = 1; segment = seg_ss; cont = 1; break;
            case 0x37: aaa(); break;

            // CMP
            case 0x38: fetchea(); setsub8 (geteab(), getr8 (cpu_reg)); break;
            case 0x39: fetchea(); setsub16(geteaw(), getr16(cpu_reg)); break;
            case 0x3A: fetchea(); setsub8 (getr8 (cpu_reg), geteab()); break;
            case 0x3B: fetchea(); setsub16(getr16(cpu_reg), geteaw()); break;
            case 0x3C: setsub8 (regs[REG_AL], FETCH()); break;
            case 0x3D: setsub16(regs[REG_AX], FETCH()); break;
            case 0x3E: sel_seg = 1; segment = seg_ds; cont = 1; break;
            case 0x3F: aas(); break;

            // INC r16
            case 0x40: case 0x41: case 0x42: case 0x43:
            case 0x44: case 0x45: case 0x46: case 0x47:

                regs[opcode&7] = setadd16nc(regs[opcode&7], 1);
                break;

            // DEC r16
            case 0x48: case 0x49: case 0x4A: case 0x4B:
            case 0x4C: case 0x4D: case 0x4E: case 0x4F:

                regs[opcode&7] = setsub16nc(regs[opcode&7], 1);
                break;

            // PUSH r16
            case 0x50: case 0x51: case 0x52: case 0x53:
            case 0x54: case 0x55: case 0x56: case 0x57:

                push(regs[opcode&7]);
                break;

            // POP r16
            case 0x58: case 0x59: case 0x5A: case 0x5B:
            case 0x5C: case 0x5D: case 0x5E: case 0x5F:

                regs[opcode&7] = pop();
                break;

            // 70-7f Условные переходы

            // MOV r8, #8
            case 0xb0: case 0xb1: case 0xb2: case 0xb3:
            case 0xb4: case 0xb5: case 0xb6: case 0xb7:

                setr8(opcode&7, FETCH());
                break;

            // MOV r16, #16
            case 0xb8: case 0xb9: case 0xba: case 0xbb:
            case 0xbc: case 0xbd: case 0xbe: case 0xbf:

                regs[opcode&7] = getword();
                break;

            default:

                // Undefined Opcode
                break;

        }
    }
    while (cont);
}
