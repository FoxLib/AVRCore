
// ---------------------------------------------------------------------
// Запись в регистры или память
// ---------------------------------------------------------------------

always @(posedge clock) if (locked) begin

    if (st == 0) latch <= ir;

    // Обычно счетчики инкрементируются
    pc <= pcstop ? pc : (pc + 1);
    st <= stnext;

    if (reg_w)  r[ reg_id ] <= res;
    if (sreg_w) sreg <= flags;

    // Автоинкремент или декремент X,Y,Z
    if (reg_ww) begin

        case (reg_idw)

            0: {r[25], r[24]} <= reg_ws ? resw : wb2; // W
            1: {r[27], r[26]} <= reg_ws ? resw : wb2; // X
            2: {r[29], r[28]} <= reg_ws ? resw : wb2; // Y
            3: {r[31], r[30]} <= reg_ws ? resw : wb2; // Z

        endcase

    end

    // Инструкции MUL
    if (reg_wm) begin

        {r[1], r[0]} <= resw;

    end

    // Запись в память на следующем такте
    o_data <= wdata;
    addr_w <= cursor;
    we     <= wren;

    // Специальные системные регистры
    if (wren)
    case (addr_w)

        // Системные регистры
        16'h005B: rampz         <= wdata; // Верхняя память ROM
        16'h005D: sp[ 7:0]      <= wdata; // SPL
        16'h005E: sp[15:8]      <= wdata; // SPH
        16'h005F: sreg          <= wdata; // SREG

        // Запись в регистры как в память
        default:  if (address < 16'h20) r[ address[4:0] ] <= wdata;

    endcase

end
