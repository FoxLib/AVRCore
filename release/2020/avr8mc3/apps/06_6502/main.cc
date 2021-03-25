#include <avr/pgmspace.h>
#define brk asm volatile("sleep");

// Операнды
enum EnumOperands { ___ = 0,
    NDX, // (b8,X)
    ZP,  // b8
    IMM, // #b8
    ABS, // b16
    NDY, // (b8),Y
    ZPX, // b8,X
    ABY, // b16,Y
    ABX, // b16,X
    REL, // b8 (адрес)
    ACC, // A
    IMP, // -- нет --
    ZPY, // b8,Y
    IND  // (b16)
};

// Инструкциия
enum EnumInstruction {

    // Базовые
    IBRK = 1,   ORA, IAND, EOR, IADC, STA, LDA,
    ICMP,  SBC, BPL,  BMI, BVC,  BVS, BCC, BCS,
     BNE,  BEQ, JSR,  RTI, RTS,  LDY, CPY, CPX,
     ASL,  PHP, CLC,  BIT, ROL,  PLP, SEC, LSR,
     PHA,  PLA, JMP,  CLI, ROR,  SEI, STY, STX,
     DEY,  TXA, TYA,  TXS, LDX,  TAY, TAX, CLV,
     TSX, IDEC, INY,  DEX, CLD, IINC, INX, NOP,

    // Расширенные
    SED, AAC, SLO, RLA, RRA, SRE, DCP, ISC, LAX,
    AAX, ASR, ARR, ATX, AXS, XAA, AXA, SYA, SXA,
    DOP
};

// Имена инструкции
const unsigned char opcode_names[256]  PROGMEM = {

    /*        00   01   02   03   04   05     06   07   08    09   0A   0B   0C    0D    0E   0F */
    /* 00 */ IBRK, ORA, ___, SLO, DOP,  ORA,  ASL, SLO, PHP,  ORA, ASL, AAC, DOP,  ORA,  ASL, SLO,
    /* 10 */ BPL,  ORA, ___, SLO, DOP,  ORA,  ASL, SLO, CLC,  ORA, NOP, SLO, DOP,  ORA,  ASL, SLO,
    /* 20 */ JSR, IAND, ___, RLA, BIT, IAND,  ROL, RLA, PLP, IAND, ROL, AAC, BIT, IAND,  ROL, RLA,
    /* 30 */ BMI, IAND, ___, RLA, DOP, IAND,  ROL, RLA, SEC, IAND, NOP, RLA, DOP, IAND,  ROL, RLA,
    /* 40 */ RTI,  EOR, ___, SRE, DOP,  EOR,  LSR, SRE, PHA,  EOR, LSR, ASR, JMP,  EOR,  LSR, SRE,
    /* 50 */ BVC,  EOR, ___, SRE, DOP,  EOR,  LSR, SRE, CLI,  EOR, NOP, SRE, DOP,  EOR,  LSR, SRE,
    /* 60 */ RTS, IADC, ___, RRA, DOP, IADC,  ROR, RRA, PLA, IADC, ROR, ARR, JMP, IADC,  ROR, RRA,
    /* 70 */ BVS, IADC, ___, RRA, DOP, IADC,  ROR, RRA, SEI, IADC, NOP, RRA, DOP, IADC,  ROR, RRA,
    /* 80 */ DOP,  STA, DOP, AAX, STY,  STA,  STX, AAX, DEY,  DOP, TXA, XAA, STY,  STA,  STX, AAX,
    /* 90 */ BCC,  STA, ___, AXA, STY,  STA,  STX, AAX, TYA,  STA, TXS, AAX, SYA,  STA,  SXA, AAX,
    /* A0 */ LDY,  LDA, LDX, LAX, LDY,  LDA,  LDX, LAX, TAY,  LDA, TAX, ATX, LDY,  LDA,  LDX, LAX,
    /* B0 */ BCS,  LDA, ___, LAX, LDY,  LDA,  LDX, LAX, CLV,  LDA, TSX, LAX, LDY,  LDA,  LDX, LAX,
    /* C0 */ CPY, ICMP, DOP, DCP, CPY, ICMP, IDEC, DCP, INY, ICMP, DEX, AXS, CPY, ICMP, IDEC, DCP,
    /* D0 */ BNE, ICMP, ___, DCP, DOP, ICMP, IDEC, DCP, CLD, ICMP, NOP, DCP, DOP, ICMP, IDEC, DCP,
    /* E0 */ CPX,  SBC, DOP, ISC, CPX,  SBC, IINC, ISC, INX,  SBC, NOP, SBC, CPX,  SBC, IINC, ISC,
    /* F0 */ BEQ,  SBC, ___, ISC, DOP,  SBC, IINC, ISC, SED,  SBC, NOP, ISC, DOP,  SBC, IINC, ISC
};

// Типы операндов для каждого опкода
const unsigned char operand_types[256] PROGMEM = {

    /*       00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F */
    /* 00 */ IMP, NDX, ___, NDX, ZP , ZP , ZP , ZP , IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
    /* 10 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    /* 20 */ ABS, NDX, ___, NDX, ZP , ZP , ZP , ZP , IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
    /* 30 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    /* 40 */ IMP, NDX, ___, NDX, ZP , ZP , ZP , ZP , IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
    /* 50 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    /* 60 */ IMP, NDX, ___, NDX, ZP , ZP , ZP , ZP , IMP, IMM, ACC, IMM, IND, ABS, ABS, ABS,
    /* 70 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    /* 80 */ IMM, NDX, IMM, NDX, ZP , ZP , ZP , ZP , IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
    /* 90 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABX,
    /* A0 */ IMM, NDX, IMM, NDX, ZP , ZP , ZP , ZP , IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
    /* B0 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
    /* C0 */ IMM, NDX, IMM, NDX, ZP , ZP , ZP , ZP , IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
    /* D0 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    /* E0 */ IMM, NDX, IMM, NDX, ZP , ZP , ZP , ZP , IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
    /* F0 */ REL, NDY, ___, NDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX
};

// Количество циклов на опкод
const unsigned char cycles_basic[256]  PROGMEM = {

      7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
      6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
      6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
      6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
      2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
      2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
      2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
      2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
      2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
      2, 6, 3, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
      2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

// ---------------------------------------------------------------------
class CPU {

protected:

    // Регистры процессора
    unsigned char  reg_A, reg_X, reg_Y, reg_S, reg_P;
    unsigned short reg_PC;
    int  cycles_ext;
    int  fatal;

public:

    CPU();

    int  get_effective(int);
    int  do_branch(int&, int);
    void do_brk();
    int  step();

    // Флаги
    void SET_ZERO(int x)        { reg_P = x ? (reg_P & 0xFD) : (reg_P | 0x02); }
    void SET_OVERFLOW(int x)    { reg_P = x ? (reg_P | 0x40) : (reg_P & 0xBF); }
    void SET_CARRY(int x)       { reg_P = x ? (reg_P | 0x01) : (reg_P & 0xFE); }
    void SET_IDECIMAL(int x)     { reg_P = x ? (reg_P | 0x08) : (reg_P & 0xF7); }
    void SET_BREAK(int x)       { reg_P = x ? (reg_P | 0x10) : (reg_P & 0xEF); }
    void SET_INTERRUPT(int x)   { reg_P = x ? (reg_P | 0x04) : (reg_P & 0xFB); }
    void SET_SIGN(int x)        { reg_P = (x & 0x80) ? (reg_P | 0x80) : (reg_P & 0x7F); };

    int  IF_CARRY()             { return (reg_P & 0x01 ? 1 : 0); }
    int  IF_ZERO()              { return (reg_P & 0x02 ? 1 : 0); }
    int  IF_INTERRUPT()         { return (reg_P & 0x04 ? 1 : 0); }
    int  IF_OVERFLOW()          { return (reg_P & 0x40 ? 1 : 0); }
    int  IF_SIGN()              { return (reg_P & 0x80 ? 1 : 0); }

    // Стек
    void PUSH(int x)     { write_byte(0x100 + reg_S, x & 0xff); reg_S = ((reg_S - 1) & 0xff); }
    unsigned char PULL() { reg_S = (reg_S + 1) & 0xff; return read_byte(0x100 + reg_S); }

    // Работа с памятью
    volatile unsigned char
        read_byte (unsigned int addr);
    volatile void
        write_byte(unsigned int addr, unsigned char data);

    unsigned short read_word (unsigned int addr) { return read_byte(addr) + (read_byte(addr) << 8); }

};

// Инициализация процессора
CPU::CPU() {

    reg_A  = 0x00;
    reg_X  = 0x00;
    reg_Y  = 0x00;
    reg_S  = 0x00;
    reg_P  = 0x00;
    reg_PC = 0xc000;
    fatal  = 0;
}

// Получение эффективного адреса
int CPU::get_effective(int addr) {

    int opcode, iaddr;
    int tmp, rt, pt;

    // Чтение опкода
    opcode = read_byte(addr++); addr &= 0xffff;

    // Разобрать операнд
    switch (operand_types[ opcode ]) {

        // PEEK( PEEK( (arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256
        // Indirect, X (b8,X)
        case NDX: {

            tmp = read_byte( addr );
            tmp = (tmp + reg_X) & 0xff;

            return read_byte( tmp ) + ((read_byte((1 + tmp) & 0xff) << 8));
        }

        // Indirect, Y (b8),Y
        case NDY: {

            tmp = read_byte(addr);
            rt  = read_byte(0xff & tmp);
            rt |= read_byte(0xff & (tmp + 1)) << 8;
            pt  = rt;
            rt  = (rt + reg_Y) & 0xffff;

            if ((pt & 0xff00) != (rt & 0xff00))
                cycles_ext++;

            return rt;
        }

        // Zero Page
        case ZP:  return read_byte( addr );

        // Zero Page, X
        case ZPX: return (read_byte(addr) + reg_X) & 0x00ff;

        // Zero Page, Y
        case ZPY: return (read_byte(addr) + reg_Y) & 0x00ff;

        // Absolute
        case ABS: return read_word(addr);

        // Absolute, X
        case ABX: {

            pt = read_word(addr);
            rt = pt + reg_X;

            if ((pt & 0xff00) != (rt & 0xff00))
                cycles_ext++;

            return rt & 0xffff;
        }

        // Absolute, Y
        case ABY: {

            pt = read_word(addr);
            rt = pt + reg_Y;

            if ((pt & 0xff00) != (rt & 0xff00))
                cycles_ext++;

            return rt & 0xffff;
        }

        // Indirect
        case IND: {

            addr  = read_word(addr);
            iaddr = read_byte(addr) + 256*read_byte((addr & 0xFF00) + ((addr + 1) & 0x00FF));
            return iaddr;
        }

        // Relative
        case REL: {

            iaddr = read_byte(addr);
            return (iaddr + addr + 1 + (iaddr < 128 ? 0 : -256)) & 0xffff;
        }
    }

    return -1;
}

// Перехо на новый адрес
int CPU::do_branch(int& addr, int iaddr) {

    if ((addr & 0xff00) != (iaddr & 0xff00)) {
        addr = iaddr;
        return 2;
    }

    addr = iaddr;
    return 1;
}

// Исполнить переход по BRK
void CPU::do_brk() {

    PUSH((reg_PC >> 8) & 0xff);      /* Вставка обратного адреса в стек */
    PUSH(reg_PC & 0xff);
    SET_BREAK(1);                    /* Установить BFlag перед вставкой */
    reg_P |= 0b00100000;             /* 1 */
    PUSH(reg_P);
    SET_INTERRUPT(1);
}

// Исполнение шага инструкции
int CPU::step() {

    int temp, optype, opname, ppurd = 1, src = 0;
    int addr = reg_PC, opcode;
    int cycles_per_instr = 2;

    // Доп. циклы разбора адреса
    cycles_ext = 0;

    // Определение эффективного адреса
    int iaddr = get_effective(addr);

    // Прочесть информацию по опкодам
    opcode = read_byte(addr) & 0xff;
    optype = pgm_read_byte( &(operand_types[ opcode ]) );
    opname = pgm_read_byte( &(opcode_names [ opcode ]) );

    // Эти инструкции НЕ ДОЛЖНЫ читать что-либо из памяти перед записью
    if (opname == STA || opname == STX || opname == STY) {
        ppurd = 0;
    }

    // Инкремент адреса при чтении опкода
    addr = (addr + 1) & 0xffff;

    // Базовые циклы + доп. циклы
    cycles_per_instr = pgm_read_byte(&(cycles_basic[ opcode ])) + cycles_ext;

    // --------------------------------
    // Чтение операнда из памяти
    // --------------------------------

    switch (optype) {

        case ___: /* printf("Opcode %02x error at %04x\n", opcode, reg_PC); exit(0); */ break;
        case NDX: // Indirect X (b8,X)
        case NDY: // Indirect, Y
        case ZP:  // Zero Page
        case ZPX: // Zero Page, X
        case ZPY: // Zero Page, Y
        case REL: // Relative

            addr = (addr + 1) & 0xffff;
            if (ppurd) src = read_byte( iaddr );
            break;

        case ABS: // Absolute
        case ABX: // Absolute, X
        case ABY: // Absolute, Y
        case IND: // Indirect

            addr = (addr + 2) & 0xffff;
            if (ppurd) src = read_byte( iaddr );
            break;

        case IMM: // Immediate

            if (ppurd) src = read_byte(addr);
            addr = (addr + 1) & 0xffff;
            break;

        case ACC: // Accumulator source

            src = reg_A;
            break;
    }

    // --------------------------------
    // Разбор инструкции и исполнение
    // --------------------------------

    switch (opname) {

        // Сложение с учетом переноса
        case IADC: {

            temp = src + reg_A + (reg_P & 1);
            SET_ZERO(temp & 0xff);
            SET_SIGN(temp);
            SET_OVERFLOW(((reg_A ^ src ^ 0x80) & 0x80) && ((reg_A ^ temp) & 0x80) );
            SET_CARRY(temp > 0xff);
            reg_A = temp & 0xff;
            break;
        }

        // Логическое умножение
        case IAND: {

            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        // Логический сдвиг вправо
        case ASL: {

            SET_CARRY(src & 0x80);
            src <<= 1;
            src &= 0xff;
            SET_SIGN(src);
            SET_ZERO(src);

            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);
            break;
        }

        // Условный переходы
        case BCC: if (!IF_CARRY())      cycles_per_instr += do_branch(addr, iaddr); break;
        case BCS: if ( IF_CARRY())      cycles_per_instr += do_branch(addr, iaddr); break;
        case BNE: if (!IF_ZERO())       cycles_per_instr += do_branch(addr, iaddr); break;
        case BEQ: if ( IF_ZERO())       cycles_per_instr += do_branch(addr, iaddr); break;
        case BPL: if (!IF_SIGN())       cycles_per_instr += do_branch(addr, iaddr); break;
        case BMI: if ( IF_SIGN())       cycles_per_instr += do_branch(addr, iaddr); break;
        case BVC: if (!IF_OVERFLOW())   cycles_per_instr += do_branch(addr, iaddr); break;
        case BVS: if ( IF_OVERFLOW())   cycles_per_instr += do_branch(addr, iaddr); break;

        // Копированиь бит 6 в OVERFLOW флаг
        case BIT: {

            SET_SIGN(src);
            SET_OVERFLOW(0x40 & src);
            SET_ZERO(src & reg_A);
            break;
        }

        // Программное прерывание
        case IBRK: {

            reg_PC = (reg_PC + 2) & 0xffff;
            do_brk();
            addr = read_word(0xFFFE);
            break;
        }

        /* Флаги */
        case CLC: SET_CARRY(0); break;
        case SEC: SET_CARRY(1); break;
        case CLD: SET_IDECIMAL(0); break;
        case SED: SET_IDECIMAL(1); break;
        case CLI: SET_INTERRUPT(0); break;
        case SEI: SET_INTERRUPT(1); break;
        case CLV: SET_OVERFLOW(0); break;

        /* Сравнение A, X, Y с операндом */
        case ICMP:
        case CPX:
        case CPY: {

            src = (opname == ICMP ? reg_A : (opname == CPX ? reg_X : reg_Y)) - src;
            SET_CARRY(src >= 0);
            SET_SIGN(src);
            SET_ZERO(src & 0xff);
            break;
        }

        /* Уменьшение операнда на единицу */
        case IDEC: {

            src = (src - 1) & 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            write_byte(iaddr, src);
            break;
        }

        /* Уменьшение X на единицу */
        case DEX: {

            reg_X = (reg_X - 1) & 0xff;
            SET_SIGN(reg_X);
            SET_ZERO(reg_X);
            break;
        }

        /* Уменьшение Y на единицу */
        case DEY: {

            reg_Y = (reg_Y - 1) & 0xff;
            SET_SIGN(reg_Y);
            SET_ZERO(reg_Y);
            break;
        }

        /* Исключающее ИЛИ */
        case EOR: {

            src ^= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        /* Увеличение операнда на единицу */
        case IINC: {

            src = (src + 1) & 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            write_byte(iaddr, src);
            break;
        }

        /* Уменьшение X на единицу */
        case INX: {

            reg_X = (reg_X + 1) & 0xff;
            SET_SIGN(reg_X);
            SET_ZERO(reg_X);
            break;
        }

        /* Увеличение Y на единицу */
        case INY: {

            reg_Y = (reg_Y + 1) & 0xff;
            SET_SIGN(reg_Y);
            SET_ZERO(reg_Y);
            break;
        }

        /* Переход по адресу */
        case JMP: addr = iaddr; break;

        /* Вызов подпрограммы */
        case JSR: {

            addr = (addr - 1) & 0xffff;
            PUSH((addr >> 8) & 0xff);   /* Вставка обратного адреса в стек (-1) */
            PUSH(addr & 0xff);
            addr = iaddr;
            break;
        }

        /* Загрузка операнда в аккумулятор */
        case LDA: {

            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = (src);
            break;
        }

        /* Загрузка операнда в X */
        case LDX: {

            SET_SIGN(src);
            SET_ZERO(src);
            reg_X = (src);
            break;
        }

        /* Загрузка операнда в Y */
        case LDY: {

            SET_SIGN(src);
            SET_ZERO(src);
            reg_Y = (src);
            break;
        }

        /* Логический сдвиг вправо */
        case LSR: {

            SET_CARRY(src & 0x01);
            src >>= 1;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);
            break;
        }

        /* Логическое побитовое ИЛИ */
        case ORA: {

            src |= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        /* Стек */
        case PHA: PUSH(reg_A); break;
        case PHP: PUSH((reg_P | 0x30)); break;
        case PLP: reg_P = PULL(); break;

        /* Извлечение из стека в A */
        case PLA: {

            src = PULL();
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        /* Циклический сдвиг влево */
        case ROL: {

            src <<= 1;
            if (IF_CARRY()) src |= 0x1;
            SET_CARRY(src > 0xff);
            src &= 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);
            break;
        }

        /* Циклический сдвиг вправо */
        case ROR: {

            if (IF_CARRY()) src |= 0x100;
            SET_CARRY(src & 0x01);
            src >>= 1;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);
            break;
        }

        /* Возврат из прерывания */
        case RTI: {

            reg_P = PULL();
            src   = PULL();
            src  |= (PULL() << 8);
            addr  = src;
            break;
        }

        /* Возврат из подпрограммы */
        case RTS: {

            src  = PULL();
            src += ((PULL()) << 8) + 1;
            addr = (src);
            break;
        }

        /* Вычитание */
        case SBC: {

            temp = reg_A - src - (IF_CARRY() ? 0 : 1);

            SET_SIGN(temp);
            SET_ZERO(temp & 0xff);
            SET_OVERFLOW(((reg_A ^ temp) & 0x80) && ((reg_A ^ src) & 0x80));
            SET_CARRY(temp >= 0);
            reg_A = (temp & 0xff);
            break;
        }

        /* Запись содержимого A,X,Y в память */
        case STA: write_byte(iaddr, reg_A); break;
        case STX: write_byte(iaddr, reg_X); break;
        case STY: write_byte(iaddr, reg_Y); break;

        /* Пересылка содержимого аккумулятора в регистр X */
        case TAX: {

            src = reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_X = (src);
            break;
        }

        /* Пересылка содержимого аккумулятора в регистр Y */
        case TAY: {

            src = reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_Y = (src);
            break;
        }

        /* Пересылка содержимого S в регистр X */
        case TSX: {

            src = reg_S;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_X = (src);
            break;
        }

        /* Пересылка содержимого X в регистр A */
        case TXA: {

            src = reg_X;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = (src);
            break;
        }

        /* Пересылка содержимого X в регистр S */
        case TXS: reg_S = reg_X; break;

        /* Пересылка содержимого Y в регистр A */
        case TYA: {

            src = reg_Y;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = (src);
            break;
        }

        // -------------------------------------------------------------
        // Недокументированные инструкции
        // -------------------------------------------------------------

        case SLO: {

            /* ASL */
            SET_CARRY(src & 0x80);
            src <<= 1;
            src &= 0xff;
            SET_SIGN(src);
            SET_ZERO(src);

            if (optype == ACC) reg_A = src;
            else write_byte(iaddr, src);

            /* ORA */
            src |= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        case RLA: {

            /* ROL */
            src <<= 1;
            if (IF_CARRY()) src |= 0x1;
            SET_CARRY(src > 0xff);
            src &= 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);

            /* IAND */
            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;
            break;
        }

        case RRA: {

            /* ROR */
            if (IF_CARRY()) src |= 0x100;
            SET_CARRY(src & 0x01);
            src >>= 1;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);

            /* IADC */
            temp = src + reg_A + (reg_P & 1);
            SET_ZERO(temp & 0xff);
            SET_SIGN(temp);
            SET_OVERFLOW(((reg_A ^ src ^ 0x80) & 0x80) && ((reg_A ^ temp) & 0x80) );
            SET_CARRY(temp > 0xff);
            reg_A = temp & 0xff;
            break;

        }

        case SRE: {

            /* LSR */
            SET_CARRY(src & 0x01);
            src >>= 1;
            SET_SIGN(src);
            SET_ZERO(src);
            if (optype == ACC) reg_A = src; else write_byte(iaddr, src);

            /* EOR */
            src ^= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;

            break;
        }

        case DCP: {

            /* IDEC */
            src = (src - 1) & 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            write_byte(iaddr, src);

            /* ICMP */
            src = reg_A - src;
            SET_CARRY(src >= 0);
            SET_SIGN(src);
            SET_ZERO(src & 0xff);
            break;
        }

        // Увеличить на +1 и вычесть из A полученное значение
        case ISC: {

            /* IINC */
            src = (src + 1) & 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            write_byte(iaddr, src);

            /* SBC */
            temp = reg_A - src - (IF_CARRY() ? 0 : 1);

            SET_SIGN(temp);
            SET_ZERO(temp & 0xff);
            SET_OVERFLOW(((reg_A ^ temp) & 0x80) && ((reg_A ^ src) & 0x80));
            SET_CARRY(temp >= 0);
            reg_A = (temp & 0xff);
            break;
        }

        // A,X = src
        case LAX: {

            reg_A = (src);
            SET_SIGN(src);
            SET_ZERO(src);
            reg_X = (src);
            break;
        }

        case AAX: write_byte(iaddr, reg_A & reg_X); break;

        // IAND + Carry
        case AAC: {

            /* IAND */
            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;

            /* Carry */
            SET_CARRY(reg_A & 0x80);
            break;
        }

        case ASR: {

            /* IAND */
            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;

            /* LSR A */
            SET_CARRY(reg_A & 0x01);
            reg_A >>= 1;
            SET_SIGN(reg_A);
            SET_ZERO(reg_A);
            break;
        }

        case ARR: {

            /* IAND */
            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;

            /* P[6] = A[6] ^ A[7]: Переполнение */
            SET_OVERFLOW((reg_A ^ (reg_A >> 1)) & 0x40);

            temp = (reg_A >> 7) & 1;
            reg_A >>= 1;
            reg_A |= (reg_P & 1) << 7;

            SET_CARRY(temp);
            SET_SIGN(reg_A);
            SET_ZERO(reg_A);
            break;
        }

        case ATX: {

            reg_A |= 0xFF;

            /* IAND */
            src &= reg_A;
            SET_SIGN(src);
            SET_ZERO(src);
            reg_A = src;

            reg_X = reg_A;
            break;

        }

        case AXS: {

            temp = (reg_A & reg_X) - src;
            SET_SIGN(temp);
            SET_ZERO(temp);
            SET_CARRY(((temp >> 8) & 1) ^ 1);
            reg_X = temp;
            break;
        }

        // Работает правильно, а тесты все равно не проходят эти 2
        case SYA: {

            temp = read_byte(reg_PC + 2);
            temp = ((temp + 1) & reg_Y);
            write_byte(iaddr, temp & 0xff);
            break;
        }

        case SXA: {

            temp = read_byte(reg_PC + 2);
            temp = ((temp + 1) & reg_X);
            write_byte(iaddr, temp & 0xff);
            break;
        }
    }

    // Установка нового адреса
    reg_PC = addr;

    return cycles_per_instr;
}

// Реализовать виртуальные функции
// ---------------------------------------------------------------------
volatile unsigned char CPU::read_byte(unsigned int addr) {
    return 0;
}

volatile void CPU::write_byte(unsigned int addr, unsigned char) {
}
// ---------------------------------------------------------------------

// Шаблон
int main() {

    CPU cpu;

    // Бесконечный цикл
    for(;;) {

        cpu.step();
    }
}
