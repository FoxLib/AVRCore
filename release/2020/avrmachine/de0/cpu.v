module cpu
(
    input  wire        clock,

    // Программная память
    output reg  [15:0] pc,          // Программный счетчик
    input  wire [15:0] ir,          // Инструкция из памяти

    // Оперативная память
    output reg  [15:0] address,     // Указатель на память RAM (sram)
    input  wire [ 7:0] din_raw,     // memory[ address ]
    output reg  [ 7:0] wb,          // Запись в память по address
    output reg         w,           // Разрешение записи в память
    output reg  [ 7:0] bank,        // Банк памяти
    output reg  [ 1:0] vmode,       // Видеорежим

    // Ввод-вывод
    input  wire [ 7:0] kb_ch,       // Клавиатура
    input  wire        kb_tr,       // Триггер внешний
    output wire        kb_hit,

    // Положение курсора
    output reg  [ 7:0] cursor_x,
    output reg  [ 7:0] cursor_y,

    // Мышь
    input  wire [ 8:0] mouse_x,
    input  wire [ 7:0] mouse_y,
    input  wire [ 1:0] mouse_cmd,

    // SPI
    output reg         spi_sent,     // =1 Сообщение отослано на spi
    output reg  [ 1:0] spi_cmd,     // Команда
    input  wire [ 7:0] spi_din,     // Принятое сообщение
    output reg  [ 7:0] spi_out,     // Сообщение на отправку
    input  wire [ 1:0] spi_st,      // bit 1: timeout (1); bit 0: busy

    // SDRAM
    output reg  [31:0] sdram_address,
    input  wire [ 7:0] sdram_i_data,
    output reg  [ 7:0] sdram_o_data,
    input  wire [ 7:0] sdram_status,
    output reg  [ 7:0] sdram_control
);

`include "core_decl.v"
`include "core_mem.v"
`include "core_timer.v"
`include "core_wb.v"

// Исполнительное устройство
always @(posedge clock)
begin if (!locked) begin

    w      <= 1'b0;
    aread  <= 1'b0;
    reg_w  <= 1'b0;
    sreg_w <= 1'b0;
    sp_mth <= 1'b0; // Ничего не делать с SP
    reg_ww <= 1'b0; // Ничего не делать с X,Y,Z
    reg_ws <= 1'b0; // Источник регистр wb2

    if (tstate == 0) latch <= ir;

    // Код пропуска инструкции (JMP, CALL, LDS, STS)
    if (skip_instr) begin

        casex (opcode)

            16'b1001_010x_xxxx_11xx, // CALL | JMP
            16'b1001_00xx_xxxx_0000: // LDS  | STS
                pc <= pcnext + 1;
            default:
                pc <= pcnext;

        endcase

        skip_instr <= 0;

    end

    // Вызов прерывания таймера
    // -----------------------------------------------------------------
    // Таймер вызван
    else if (intr_trigger) begin

        case (tstate)

            // Запись PCL
            0: begin

                tstate  <= 1;
                address <= sp;
                wb      <= pc[7:0];
                w       <= 1'b1;
                sp_mth  <= `SPDEC;

            end

            // Запись PCH
            1: begin

                tstate  <= 0;
                address <= sp;
                wb      <= pcnext[15:8];
                w       <= 1'b1;
                sp_mth  <= `SPDEC;
                pc      <= 2;           // ISR(INT0_vect)
                intr_trigger <= 0;      // Переход к обычному исполнению

                // Сброс флага I->0 (sreg)
                alu     <= 11;
                op2     <= {1'b0, sreg[6:0]};
                sreg_w  <= 1'b1;

            end

        endcase

    end

    // Есть истечение времени работы инструкции
    else if (tstate == 0 && sreg[7] && (timer_ms[7:0] - intr_timer > intr_maxtime)) begin

        intr_trigger <= 1'b1;
        intr_timer   <= timer_ms[7:0];

    end
    // -----------------------------------------------------------------

    // Исполнение опкодов
    else casex (opcode)

        // NOP, BREAK
        16'b0000_0000_0000_0000: pc <= pcnext;
        16'b1001_0101_1001_1000: pc <= pcnext;

        // [1T] LDI Rd, K
        16'b1110_xxxx_xxxx_xxxx: begin

            pc      <= pcnext;
            alu     <= 0;       // Инструкция LDI
            op2     <= K;
            reg_w   <= 1'b1;
            reg_id  <= rdi;

        end

        // [1T] RJMP k | IJMP | EIJMP
        16'b1100_xxxx_xxxx_xxxx: pc <= pcnext + {{4{opcode[11]}}, opcode[11:0]};
        16'b1001_0100_000x_1001: pc <= Z;

        // [2T] JMP k
        16'b1001_010x_xxxx_110x:
        case (tstate)

            0: begin tstate <= 1; pc <= pcnext; end
            1: begin tstate <= 0; pc <= ir; end

        endcase

        // [1T] <ALU> Rd, Rr
        16'b000x_01xx_xxxx_xxxx, // 1 CPC, 5 CP
        16'b000x_1xxx_xxxx_xxxx, // 2 SBC, 3 ADD, 6 SUB, 7 ADC
        16'b0010_0xxx_xxxx_xxxx, // 8 AND, 9 EOR
        16'b0010_10xx_xxxx_xxxx: // A OR
        begin

            pc      <= pcnext;
            alu     <= opcode[13:10];
            op1     <= r[rd];
            op2     <= r[rr];
            // CP, CPС не писать
            reg_id  <= rd;
            reg_w   <= (opcode[13:10] != 4'b0001 && opcode[13:10] != 4'b0101);
            sreg_w  <= 1'b1;

        end

        // [1T] BRB[C/S] k
        16'b1111_0xxx_xxxx_xxxx:
        begin

            if (sreg[ opcode[2:0] ] ^ opcode[10])
                pc <= pcnext + {{9{opcode[9]}}, opcode[9:3]};
            else
                pc <= pcnext;

        end

        // [1T] АЛУ с непосредственным операндом
        16'b0011_xxxx_xxxx_xxxx, // CPI
        16'b0100_xxxx_xxxx_xxxx, // SBCI
        16'b0101_xxxx_xxxx_xxxx, // SUBI
        16'b0110_xxxx_xxxx_xxxx, // ORI
        16'b0111_xxxx_xxxx_xxxx: // ANDI
        begin

            pc      <= pcnext;
            op1     <= r[rdi];
            op2     <= K;
            reg_id  <= rdi;
            reg_w   <= (opcode[15:12] != 4'b0011); // CPI не писать
            sreg_w  <= 1'b1;

            case (opcode[15:12])
                4'b0011: alu <= 5;  // CPI
                4'b0100: alu <= 2;  // SBCI
                4'b0101: alu <= 6;  // SUBI
                4'b0110: alu <= 10; // ORI
                4'b0111: alu <= 8;  // ANDI
            endcase

        end

        // [1T] MOV Rd, Rr
        16'b0010_11xx_xxxx_xxxx:
        begin

            pc      <= pcnext;
            alu     <= 0;
            op2     <= r[rr];
            reg_id  <= rd;
            reg_w   <= 1'b1;

        end

        // [2T] RCALL k | ICALL | EICALL | CALL
        16'b1101_xxxx_xxxx_xxxx,
        16'b1001_0101_000x_1001,
        16'b1001_010x_xxxx_111x:
        case (tstate)

            // Запись PCL
            0: begin

                tstate  <= 1;
                address <= sp;
                wb      <= is_call ? pcnext2[7:0] : pcnext[7:0];
                w       <= 1'b1;
                sp_mth  <= `SPDEC;
                pc      <= pcnext;

            end

            // Запись PCH
            1: begin

                tstate  <= 0;
                address <= sp;
                wb      <= is_call ? pcnext[15:8] : pc[15:8];
                w       <= 1'b1;
                sp_mth  <= `SPDEC;

                // Метод вызова
                pc      <= is_call     ? ir : // CALL
                           opcode[14]  ? (pc + {{4{opcode[11]}}, opcode[11:0]}) : // RCALL
                                         Z;   // ICALL

            end

        endcase

        // [3T] 9508 RET | 9518 RETI
        16'b1001_0101_000x_1000:
        case (tstate)

            // Указатель адреса
            0: begin

                tstate  <= 1;
                address <= sp + 1;
                sp_mth  <= `SPINC;

            end

            // Чтение PCH
            1: begin

                tstate   <= 2;
                pc[15:8] <= din;
                address  <= sp + 1;
                sp_mth   <= `SPINC;
                kbit     <= din[0];
                aread    <= 1'b1;

            end

            // Чтение PCL
            2: begin

                tstate   <= 0;
                pc[ 7:0] <= din;
                alu      <= 11;
                op2      <= {sreg[7] | opcode[4], sreg[6:0]};
                sreg_w   <= 1;
                kbit     <= din[0];
                aread    <= 1'b1;

            end

        endcase

        // [2T] LD Rd, (X|Y|Z)+
        // [1T] ST Rd, (X|Y|Z)+
        16'b1001_00xx_xxxx_1100, // X
        16'b1001_00xx_xxxx_1101, // X+
        16'b1001_00xx_xxxx_1110, // -X
        16'b1001_00xx_xxxx_1001, // Y+
        16'b1001_00xx_xxxx_1010, // -Y
        16'b1001_00xx_xxxx_0001, // Z+
        16'b1001_00xx_xxxx_0010: // -Z
        case (tstate)

            // Установка указателя на память
            0: begin

                tstate <= opcode[9] ? 0 : 1;
                pc     <= pcnext;
                wb     <= r[rd];
                w      <= opcode[9];

                // Выбор адреса
                case (opcode[3:0])

                    4'b11_00: begin address <= X;  end
                    4'b11_01: begin address <= X;  wb2 <= Xp; reg_idw <= 2'b01; reg_ww <= 1; end
                    4'b11_10: begin address <= Xm; wb2 <= Xm; reg_idw <= 2'b01; reg_ww <= 1; end
                    4'b10_01: begin address <= Y;  wb2 <= Yp; reg_idw <= 2'b10; reg_ww <= 1; end
                    4'b10_10: begin address <= Ym; wb2 <= Ym; reg_idw <= 2'b10; reg_ww <= 1; end
                    4'b00_01: begin address <= Z;  wb2 <= Zp; reg_idw <= 2'b11; reg_ww <= 1; end
                    4'b00_10: begin address <= Zm; wb2 <= Zm; reg_idw <= 2'b11; reg_ww <= 1; end

                endcase

            end

            // Запись в регистр Rd (LD)
            1: begin

                tstate  <= 0;
                alu     <= 0;
                op2     <= din;
                reg_w   <= 1;
                reg_id  <= rd;
                kbit    <= din[0];
                aread   <= 1'b1;

            end

        endcase

        // [2T] LDD Y+q, Z+q
        // [1T] STD Y+q, Z+q
        16'b10x0_xxxx_xxxx_xxxx: // Y,Z
        case (tstate)

            // Установка указателя на память
            0: begin

                tstate  <= opcode[9] ? 0 : 1;
                pc      <= pcnext;
                address <= (opcode[3] ? Y : Z) + q;
                wb      <= r[rd];
                w       <= opcode[9];

            end

            // Запись в регистр Rd
            1: begin

                tstate  <= 0;
                alu     <= 0;
                op2     <= din;
                reg_w   <= 1;
                reg_id  <= rd;
                kbit    <= din[0];
                aread   <= 1'b1;

            end

        endcase

        // [1T] 0=COM, 1=NEG, 2=SWAP, 3=INC, 5=ASR, 6=LSR, 7=ROR, 10=DEC
        16'b1001_010x_xxxx_00xx,
        16'b1001_010x_xxxx_011x,
        16'b1001_010x_xxxx_0101,
        16'b1001_010x_xxxx_1010: begin

            pc      <= pcnext;
            op1     <= r[rd];   // Rd
            reg_w   <= 1;
            sreg_w  <= 1;
            reg_id  <= rd;

            case (opcode[3:0])

                4'b0000: alu <= 5'h0C; // COM
                4'b0001: alu <= 5'h0D; // NEG
                4'b0010: alu <= 5'h0E; // SWAP
                4'b0011: alu <= 5'h0F; // INC
                4'b0101: alu <= 5'h10; // ASR
                4'b0110: alu <= 5'h11; // LSR
                4'b0111: alu <= 5'h12; // ROR
                4'b1010: alu <= 5'h13; // DEC

            endcase

        end

        // [2T] MOVW Rd, Rr
        16'b0000_0001_xxxx_xxxx:
        case (tstate)

            // LO регистр
            0: begin

                tstate  <= 1;
                pc      <= pcnext;
                alu     <= 0;
                op2     <= r[ {opcode[3:0], 1'b0} ];
                reg_id  <=    {opcode[7:4], 1'b0};
                reg_w   <= 1;

            end

            // HI регистр
            1: begin

                tstate  <= 0;
                alu     <= 0;
                op2     <= r[ {opcode[3:0], 1'b1} ];
                reg_id  <=    {opcode[7:4], 1'b1};
                reg_w   <= 1;

            end

        endcase

        // [1T] <ADIW|SBIW> Rd, K
        16'b1001_0110_xxxx_xxxx, // ADIW
        16'b1001_0111_xxxx_xxxx: // SBIW
        begin

            pc      <= pcnext;
            alu     <= opcode[8] ? 5'h15 : 5'h14;

            case (opcode[5:4])
                0: op1w <= {r[25], r[24]};
                1: op1w <= {r[27], r[26]};
                2: op1w <= {r[29], r[28]};
                3: op1w <= {r[31], r[30]};
            endcase

            op2     <= {opcode[7:6], opcode[3:0]};
            reg_idw <= opcode[5:4];
            reg_ww  <= 1;
            reg_ws  <= 1;
            sreg_w  <= 1;

        end

        // [2T] LPM Rd, Z
        16'b1001_0101_110x_1000, // LPM R0, Z  | ELPM R0, Z
        16'b1001_000x_xxxx_01x0, // LPM Rd, Z  | ELPM Rd, Z
        16'b1001_000x_xxxx_01x1: // LPM Rd, Z+ | ELPM Rd, Z+
        case (tstate)

            // Считывание байта
            0: begin

                tstate  <= 1;
                pclatch <= pcnext;

                // Если ELPM, то записать в старший бит rampz[0]
                if (opcode[1] || (opcode[10] && opcode[4]))
                     pc <= {rampz[0], Z[15:1]}; // ELPM
                else pc <= Z[15:1];             // LPM

            end

            // Запись
            1: begin

                tstate  <= 0;
                pc      <= pclatch;
                alu     <= 0;       // LDI
                reg_idw <= 2'b11;   // Z
                reg_ww  <= (!opcode[10] & opcode[0]); // Z+
                wb2     <= Zp;
                reg_w   <= 1;
                reg_id  <= opcode[10] ? 0 : rd;         // R0, Rd
                op2     <= Z[0] ? ir[15:8] : ir[7:0];   // Hi, Lo

            end


        endcase

        // [2T] IN  Rd, A
        // [1T] OUT A, Rd
        16'b1011_xxxx_xxxx_xxxx:
        case (tstate)

            // Установка адреса
            0: begin

                tstate  <= opcode[11] ? 0 : 1;
                pc      <= pcnext;
                wb      <= r[rd];
                w       <= opcode[11];
                address <= {opcode[10:9], opcode[3:0]} + 16'h20;

            end

            // Запись регистра
            1: begin

                tstate  <= 0;
                alu     <= 0;
                op2     <= din;
                reg_id  <= rd;
                reg_w   <= 1;
                kbit    <= din[0];
                aread   <= 1'b1;

            end

        endcase

        // [1T] SBR[C,S] Rd, b
        // [1T] SBRS Rd, b
        16'b1111_11xx_xxxx_0xxx: begin

            pc <= pcnext;
            if (r[rd][ opcode[2:0] ] == opcode[9])
                skip_instr <= 1;

        end

        // [2T] SBI[C,S] A, b
        // [2T] SBIS A, b
        16'b1001_10x1_xxxx_xxxx: // C=0,S=1
        casex (tstate)

            // Запрос чтения
            0: begin

                tstate  <= 1;
                address <= opcode[7:3] + 16'h20;

            end

            // Вычисление бита
            1: begin

                tstate  <= 0;
                kbit    <= din[0];
                aread   <= 1'b1;
                pc      <= pcnext;

                if (din[ opcode[2:0] ] == opcode[9])
                    skip_instr <= 1;

            end

        endcase

        // [1T] CPSE Rd, Rr
        16'b0001_00xx_xxxx_xxxx: begin

            if (r[rd] == r[rr]) skip_instr <= 1;
            pc <= pcnext;

        end

        // [3T] LDS Rd, Mem
        // [2T] STS Mem, Rd
        16'b1001_00xx_xxxx_0000: // 0=lds, 1=sts
        case (tstate)

            // К следующему коду
            0: begin

                tstate  <= 1;
                pc      <= pcnext;

            end

            // Запись в память или выбор регистра
            1: begin

                tstate  <= opcode[9] ? 0 : 2; // 1=STS, 0=LDS
                pc      <= pcnext;
                reg_id  <= rd;
                address <= ir;
                wb      <= r[rd];
                w       <= opcode[9];

            end

            // Запись в регистр
            2: begin

                tstate  <= 0;
                alu     <= 0;
                reg_w   <= 1;
                op2     <= din;
                kbit    <= din[0];
                aread   <= 1'b1;

            end

        endcase

        // [1T] BCLR, BSET
        16'b1001_0100_xxxx_1000: begin

            case (opcode[6:4])

                0: op2 <= {sreg[7:1], !opcode[7]};
                1: op2 <= {sreg[7:2], !opcode[7], sreg[0]};
                2: op2 <= {sreg[7:3], !opcode[7], sreg[1:0]};
                3: op2 <= {sreg[7:4], !opcode[7], sreg[2:0]};
                4: op2 <= {sreg[7:5], !opcode[7], sreg[3:0]};
                5: op2 <= {sreg[7:6], !opcode[7], sreg[4:0]};
                6: op2 <= {sreg[7],   !opcode[7], sreg[5:0]};
                7: op2 <= {!opcode[7],            sreg[6:0]};

            endcase

            alu     <= 11;
            sreg_w  <= 1;
            pc      <= pcnext;

        end

        // [1T] PUSH Rd
        16'b1001_001x_xxxx_1111: begin

            pc      <= pcnext;
            wb      <= r[rd];
            w       <= 1;
            address <= sp;
            sp_mth  <= `SPDEC;

        end

        // [2T] POP Rd
        16'b1001_000x_xxxx_1111:
        case (tstate)

            // Указатель адреса
            0: begin

                tstate  <= 1;
                pc      <= pcnext;
                address <= sp + 1;
                sp_mth  <= `SPINC;

            end

            // Запись в регистр
            1: begin

                tstate  <= 0;
                alu     <= 0;
                op2     <= din;
                reg_id  <= rd;
                reg_w   <= 1;
                kbit    <= din[0];
                aread   <= 1'b1;

            end

        endcase

        // [1T] BST Rd, b
        16'b1111_101x_xxxx_0xxx: begin

            pc      <= pcnext;
            alu     <= 11;
            op2     <= {sreg[7], r[rd][ opcode[2:0] ], sreg[5:0]};
            sreg_w  <= 1;

        end

        // [1T] BLD Rd, b
        16'b1111_100x_xxxx_0xxx: begin

            pc      <= pcnext;
            alu     <= 22;  // BLD
            op1     <= r[rd];
            op2     <= opcode[2:0];
            reg_id  <= rd;
            reg_w   <= 1;

        end

        // [2T] CBI / SBI A, b
        16'b1001_10x0_xxxx_xxxx:
        case (tstate)

            // Чтение из порта
            0: begin

                tstate  <= 1;
                pc      <= pcnext;
                address <= opcode[7:3] + 16'h20;

            end

            // Запись в порт
            1: begin

                tstate  <= 0;
                kbit    <= din[0];
                aread   <= 1'b1;

                case (opcode[2:0])
                    0: wb <= {din[7:1], opcode[9]};
                    1: wb <= {din[7:2], opcode[9], din[0]};
                    2: wb <= {din[7:3], opcode[9], din[1:0]};
                    3: wb <= {din[7:4], opcode[9], din[2:0]};
                    4: wb <= {din[7:5], opcode[9], din[3:0]};
                    5: wb <= {din[7:6], opcode[9], din[4:0]};
                    6: wb <= {din[  7], opcode[9], din[5:0]};
                    7: wb <= {          opcode[9], din[6:0]};
                endcase
                w <= 1;

            end

        endcase

    endcase

end
end

endmodule
