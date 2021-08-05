
// Режим работы АЛУ
// ---------------------------------------------------------------------
// 0 LDI    9  EOR      11 LSR      19 MULSU
// 1 CPC    A  OR       12 ROR
// 2 SBC    B  <SREG>   13 DEC
// 3 ADD    C  COM      14 ADIW
// 5 CP     D  NEG      15 SBIW
// 6 SUB    E  SWAP     16 BLD
// 7 ADC    F  INC      17 MUL
// 8 AND    10 ASR      18 MULS
// ---------------------------------------------------------------------

// Ввод
reg [4:0]  alu;
reg [7:0]  dst;
reg [7:0]  src;

// 16 bit
reg [15:0] op1w;
reg [15:0] resw;

// Вывод
reg [7:0]  res;
reg [7:0]  flags;
// ---------------------------------------------------------------------

// Вычисления
wire [7:0] sub = dst - src;
wire [7:0] add = dst + src;
wire [8:0] sbc = dst - src - sreg[0];
wire [7:0] adc = dst + src + sreg[0];
wire [7:0] lsr = {1'b0, dst[7:1]};
wire [7:0] ror = {sreg[0], dst[7:1]};
wire [7:0] asr = {dst[7], dst[7:1]};
wire [7:0] neg = -dst;
wire [7:0] inc = dst + 1;
wire [7:0] dec = dst - 1;
wire [7:0] com = dst ^ 8'hFF;
wire [7:0] swap = {dst[3:0], dst[7:4]};
reg        carry;

// 16 битные вычисления
wire [15:0] adiw  = op1w + src;
wire [15:0] sbiw  = op1w - src;
wire [15:0] mul   = dst[7:0] * src[7:0];
wire [15:0] mulsu = {{8{dst[7]}}, dst[7:0]} * src[7:0];
wire [15:0] muls  = {{8{dst[7]}}, dst[7:0]} * {{8{src[7]}}, src[7:0]};

// Флаги переполнения после сложения и вычитания
wire add_flag_v = (dst[7] &  src[7] & !res[7]) | (!dst[7] & !src[7] & res[7]);
wire sub_flag_v = (dst[7] & !src[7] & !res[7]) | (!dst[7] &  src[7] & res[7]);
wire neg_flag_v = res[7:0] == 8'h80;

// Флаги половинного переполнения после сложения и вычитания
wire add_flag_h = ( dst[3] & src[3]) | (src[3] & !res[3]) | (!res[3] &  dst[3]);
wire sub_flag_h = (!dst[3] & src[3]) | (src[3] &  res[3]) | ( res[3] & !dst[3]);
wire neg_flag_h = dst[3] | (dst[3] & res[3]) | res[3];

// Флаги ADIW, SBIW
wire adiw_v = !op1w[15] & resw[15];
wire adiw_c = !resw[15] & op1w[15];

// ---------------------------------------------------------------------

// Логические флаги
wire [7:0] set_logic_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ res[7],
    /* v */ 1'b0,
    /* n */ res[7],
    /* z */ res[7:0] == 0,
    /* c */ sreg[0]
};

// Флаги после вычитания с переносом
wire [7:0] set_subcarry_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sub_flag_h,
    /* s */ sub_flag_v ^ res[7],
    /* v */ sub_flag_v,
    /* n */ res[7],
    /* z */ (res[7:0] == 0) & sreg[1],
    /* c */ sbc[8]
};

// Флаги после вычитания
wire [7:0] set_subtract_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sub_flag_h,
    /* s */ sub_flag_v ^ res[7],
    /* v */ sub_flag_v,
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ dst < src
};

// Флаги после COM
wire [7:0] set_com_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ res[7],
    /* v */ 1'b0,
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ 1'b1
};

// Флаги после NEG
wire [7:0] set_neg_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ neg_flag_h,
    /* s */ neg_flag_v ^ res[7],
    /* v */ neg_flag_v,
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ dst != 0
};

// Флаги после сложения с переносом
wire [7:0] set_add_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ add_flag_h,
    /* s */ add_flag_v ^ res[7],
    /* v */ add_flag_v,
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ dst + src + carry >= 9'h100
};

// Флаги после логической операции сдвига вправо
wire [7:0] set_lsr_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ dst[0],
    /* v */ res[7] ^ dst[0],
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ dst[0]
};

// Флаги после INC
wire [7:0] set_inc_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ (res == 8'h80) ^ res[7],
    /* v */ (res == 8'h80),
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ sreg[0]
};

// Флаги после DEC
wire [7:0] set_dec_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ (res == 8'h7F) ^ res[7],
    /* v */ (res == 8'h7F),
    /* n */ res[7],
    /* z */ (res[7:0] == 0),
    /* c */ sreg[0]
};

// Флаги после ADIW
wire [7:0] set_adiw_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ adiw_v ^ resw[15],
    /* v */ adiw_v,
    /* n */ resw[15],
    /* z */ (resw[15:0] == 0),
    /* c */ adiw_c
};

// Флаги после SBIW
wire [7:0] set_sbiw_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ adiw_v ^ resw[15],
    /* v */ adiw_v,
    /* n */ resw[15],
    /* z */ (resw[15:0] == 0),
    /* c */ adiw_v
};

// Флаги после MUL
wire [7:0] set_mul_flag = {

    /* i */ sreg[7],
    /* t */ sreg[6],
    /* h */ sreg[5],
    /* s */ sreg[7],
    /* v */ sreg[7],
    /* n */ sreg[7],
    /* z */ (mul[15:0] == 0),
    /* c */ mul[15]
};

always @(*) begin

    flags = sreg;
    res   = dst;
    resw  = 0;
    carry = 0;

    case (alu)

        /* LDI   */ 0:  begin res   = src;       end
        /* CPC   */ 1:  begin res   = sbc[7:0];  flags = set_subcarry_flag; end
        /* SBC   */ 2:  begin res   = sbc[7:0];  flags = set_subcarry_flag; end
        /* ADD   */ 3:  begin res   = add;       flags = set_add_flag;      carry = 0; end
        /* CP    */ 5:  begin res   = sub;       flags = set_subtract_flag; end
        /* SUB   */ 6:  begin res   = sub;       flags = set_subtract_flag; end
        /* ADC   */ 7:  begin res   = adc;       flags = set_add_flag;      carry = sreg[0]; end
        /* AND   */ 8:  begin res   = dst & src; flags = set_logic_flag;    end
        /* EOR   */ 9:  begin res   = dst ^ src; flags = set_logic_flag;    end
        /* OR    */ 10: begin res   = dst | src; flags = set_logic_flag;    end
        /* SREG  */ 11: begin res   = dst;       flags = src;               end
        /* COM   */ 12: begin res   = com;       flags = set_com_flag;      end
        /* NEG   */ 13: begin res   = neg;       flags = set_neg_flag;      end
        /* SWAP  */ 14: begin res   = swap;      end
        /* INC   */ 15: begin res   = inc;       flags = set_inc_flag;  end
        /* ASR   */ 16: begin res   = asr;       flags = set_lsr_flag;  end
        /* LSR   */ 17: begin res   = lsr;       flags = set_lsr_flag;  end
        /* ROR   */ 18: begin res   = ror;       flags = set_lsr_flag;  end
        /* DEC   */ 19: begin res   = dec;       flags = set_dec_flag;  end
        /* ADIW  */ 20: begin resw  = adiw;      flags = set_adiw_flag; end
        /* SBIW  */ 21: begin resw  = sbiw;      flags = set_sbiw_flag; end
        /* BLD   */ 22: begin

            case (src[2:0])

                0: res = {dst[7:1], sreg[6]};
                1: res = {dst[7:2], sreg[6], dst[0]};
                2: res = {dst[7:3], sreg[6], dst[1:0]};
                3: res = {dst[7:4], sreg[6], dst[2:0]};
                4: res = {dst[7:5], sreg[6], dst[3:0]};
                5: res = {dst[7:6], sreg[6], dst[4:0]};
                6: res = {dst[  7], sreg[6], dst[5:0]};
                7: res = {          sreg[6], dst[6:0]};

            endcase

        end
        /* MUL   */ 23: begin resw = mul;   flags = set_mul_flag; end
        /* MULS  */ 24: begin resw = muls;  flags = set_mul_flag; end
        /* MULSU */ 25: begin resw = mulsu; flags = set_mul_flag; end

        default: res = 8'hFF;

    endcase

end
