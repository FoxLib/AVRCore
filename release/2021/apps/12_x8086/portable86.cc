#include "portable86.h"

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
}

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

// ---------------------------------------------------------------------
// СЧИТЫВАНИЕ И ДЕКОДИРОВАНИЕ ОПКОДА
// ---------------------------------------------------------------------

// Считывание следующего кода из CS:IP
uint8_t FETCH() { return readmemb(segs[seg_CS], ip++); }
