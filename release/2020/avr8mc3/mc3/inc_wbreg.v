reg [7:0] din;

// ---------------------------------------------------------------------
// Проксирование памяти => DIN
// ---------------------------------------------------------------------

always @* begin

    casex (address)

        // Регистры
        16'b0000_0000_000x_xxxx: din = r[ address[4:0] ];

        // Процессор
        16'h005B: din = rampz;
        16'h005D: din = sp[ 7:0];
        16'h005E: din = sp[15:8];
        16'h005F: din = sreg;

        // Память
        default:  din = din_raw;

    endcase

end

// ---------------------------------------------------------------------
// Запись в регистры
// ---------------------------------------------------------------------

// Блокировка процессора на время I/O
always @(negedge clock) if (locked) begin

    // Запись в регистры и флаги
    if (reg_w)  r[ reg_id ] <= alu_res;
    if (sreg_w)        sreg <= alu_sreg;

    // Икремент или декремент
    case (sp_mth) `SPDEC: sp <= sp - 1; `SPINC: sp <= sp + 1; endcase

    // Автоинкремент или декремент X,Y,Z
    if (reg_ww)
    case (reg_idw)

        0: {r[25], r[24]} <= reg_ws ? resw : wb2; // W
        1: {r[27], r[26]} <= reg_ws ? resw : wb2; // X
        2: {r[29], r[28]} <= reg_ws ? resw : wb2; // Y
        3: {r[31], r[30]} <= reg_ws ? resw : wb2; // Z

    endcase

    // Запись в порты или регистры
    if (w)
    case (address)

        // Системные регистры
        16'h005B: rampz     <= wb; // Верхняя память ROM
        16'h005D: sp[ 7:0]  <= wb; // SPL
        16'h005E: sp[15:8]  <= wb; // SPH
        16'h005F: sreg      <= wb; // SREG

        // Запись в регистры как в память
        default: if (address < 16'h20) r[ address[4:0] ] <= wb;

    endcase

end
