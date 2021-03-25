#include "main.h"

// Применение маски к опкоду
void APP::mask(const char* mask, int opcode) {

    int i, j;
    int mk[16], nmask = 0, baseop = 0;

    // Расчет позиции, где находится маска
    for (i = 0; i < 16; i++) {

        int ch = mask[15-i];
        if (ch == '0' || ch == '1')
            baseop |= (ch == '1' ? 1 << i : 0);
        else
            mk[nmask++] = i;
    }

    // Перебрать 2^nmask вариантов, по количеству битов и назначить всем опкод
    for (i = 0; i < (1<<nmask); i++) {

        int instn = baseop;
        for (j = 0; j < nmask; j++) if (i & (1 << j)) instn |= (1 << mk[j]);
        map[instn] = opcode;
    }
}

// Создание инструкции
void APP::instrmask() {

    int i;

    for (i = 0; i < 65536; i++) { sram[i] = i < 32 ? 0xFF : 0; program[i] = 0; }
    for (i = 0; i < 65536; i++) { map[i] = UNDEFINED; }

    sram[0x5D] = 0xFF;
    sram[0x5E] = 0xEF;
    sram[0x5F] = 0x00; // SREG

    // Для отладчика
    for (i = 0; i < 96; i++) pvsram[i] = sram[i];

    // Очистка флагов
    flag.c = 0; flag.s = 0;
    flag.z = 0; flag.h = 0;
    flag.n = 0; flag.t = 0;
    flag.v = 0; flag.i = 0;

    // Копировать DOS палитру
    for (i = 0; i < 256; i++) {

        int bc = (DOS_13[i] & 0xF0) >> 4;
        int gc = (DOS_13[i] & 0xF000) >> 8;
        int rc = (DOS_13[i] & 0xF00000) >> 20;

        sram[MEMORY_FONT_PAL + 2*i   ] = bc | gc;
        sram[MEMORY_FONT_PAL + 2*i + 1] = rc;
    }

    // Скопировать FontROM (Bank 1)
    for (i = 0; i < 4096; i++) {
        sram[MEMORY_FONT_ADDR + i] = ansi16[i >> 4][i & 15];
    }

    // Арифметические на 2 регистра
    mask("000001rdddddrrrr", CPC);   // +
    mask("000010rdddddrrrr", SBC);   // +
    mask("000011rdddddrrrr", ADD);   // +
    mask("000101rdddddrrrr", CP);    // +
    mask("000110rdddddrrrr", SUB);   // +
    mask("000111rdddddrrrr", ADC);   // +
    mask("001000rdddddrrrr", AND);   // +
    mask("001001rdddddrrrr", EOR);   // +
    mask("001010rdddddrrrr", OR);    // +

    // Арифметика регистр + операнд K
    mask("0011KKKKddddKKKK", CPI);   // +
    mask("0100KKKKddddKKKK", SBCI);  // +
    mask("0101KKKKddddKKKK", SUBI);  // +
    mask("0110KKKKddddKKKK", ORI);   // +
    mask("0111KKKKddddKKKK", ANDI);  // +
    mask("10010110KKddKKKK", ADIW);  // +
    mask("10010111KKddKKKK", SBIW);  // +

    // Условные и безусловные переходы
    mask("1100kkkkkkkkkkkk", RJMP);  // +
    mask("1101kkkkkkkkkkkk", RCALL); // +
    mask("1001010100001000", RET);   // +
    mask("1001010100011000", RETI);  // +
    mask("111100kkkkkkksss", BRBS);  // +
    mask("111101kkkkkkksss", BRBC);  // +
    mask("1111110ddddd0bbb", SBRC);  // +
    mask("1111111ddddd0bbb", SBRS);  // +
    mask("10011001AAAAAbbb", SBIC);  // +
    mask("10011011AAAAAbbb", SBIS);  // +
    mask("000100rdddddrrrr", CPSE);  // +

    // Непрямые и длинные переходы
    mask("1001010100001001", ICALL); // +
    mask("1001010100011001", EICALL);
    mask("1001010000001001", IJMP);  // +
    mask("1001010000011001", EIJMP);
    mask("1001010kkkkk111k", CALL);  // +
    mask("1001010kkkkk110k", JMP);   // +

    // Перемещения
    mask("1110KKKKddddKKKK", LDI);   // +
    mask("001011rdddddrrrr", MOV);   // +
    mask("1001000ddddd0000", LDS);   // +
    mask("1001001ddddd0000", STS);   // +
    mask("00000001ddddrrrr", MOVW);  // +
    mask("1111100ddddd0bbb", BLD);   // +
    mask("1111101ddddd0bbb", BST);   // +
    mask("1001001ddddd0100", XCH);   // +

    // Однооперандные
    mask("1001010ddddd0011", INC);   // +
    mask("1001010ddddd1010", DEC);   // +
    mask("1001010ddddd0110", LSR);   // +
    mask("1001010ddddd0101", ASR);   // +
    mask("1001010ddddd0111", ROR);   // +
    mask("1001010ddddd0001", NEG);   // +
    mask("1001010ddddd0000", COM);   // +
    mask("1001010ddddd0010", SWAP);  // +
    mask("1001001ddddd0110", LAC);   // +
    mask("1001001ddddd0101", LAS);   // +
    mask("1001001ddddd0111", LAT);   // +
    mask("100101001sss1000", BCLR);  // +
    mask("100101000sss1000", BSET);  // +
    mask("10011000AAAAAbbb", CBI);   // +
    mask("10011010AAAAAbbb", SBI);   // +

    // Косвенная загрузка из памяти
    mask("1001000ddddd1100", LDX);   // +
    mask("1001000ddddd1101", LDX_);  // +
    mask("1001000ddddd1110", LD_X);  // +
    mask("1001000ddddd1001", LDY_);  // +
    mask("1001000ddddd1010", LD_Y);  // +
    mask("10q0qq0ddddd1qqq", LDYQ);  // +
    mask("1001000ddddd0001", LDZ_);  // +
    mask("1001000ddddd0010", LD_Z);  // +
    mask("10q0qq0ddddd0qqq", LDZQ);  // +

    // Косвенное сохранение
    mask("1001001ddddd1100", STX);   // +
    mask("1001001ddddd1101", STX_);  // +
    mask("1001001ddddd1110", ST_X);  // +
    mask("1001001ddddd1001", STY_);  // +
    mask("1001001ddddd1010", ST_Y);  // +
    mask("10q0qq1ddddd1qqq", STYQ);  // +
    mask("1001001ddddd0001", STZ_);  // +
    mask("1001001ddddd0010", ST_Z);  // +
    mask("10q0qq1ddddd0qqq", STZQ);  // +

    // Загрузка из запись в память программ
    mask("1001010111001000", LPM0Z);  // +
    mask("1001000ddddd0100", LPMRZ);  // +
    mask("1001000ddddd0101", LPMRZ_); // +
    mask("1001010111101000", SPM);
    mask("1001010111111000", SPM2);

    // Особые инструкции расширенной загрузки из памяти
    mask("1001010111011000", ELPM0Z);  // +
    mask("1001000ddddd0110", ELPMRZ);  // +
    mask("1001000ddddd0111", ELPMRZ_); // +

    // Специальные
    mask("1001010110001000", SLEEP); // +
    mask("1001010110101000", WDR);   // +
    mask("1001010110011000", BREAK); // +
    mask("0000000000000000", NOP);   // +
    mask("10110AAdddddAAAA", IN);    // +
    mask("10111AAdddddAAAA", OUT);   // +
    mask("1001001ddddd1111", PUSH);  // +
    mask("1001000ddddd1111", POP);   // +
    mask("10010100KKKK1011", DES);
}
