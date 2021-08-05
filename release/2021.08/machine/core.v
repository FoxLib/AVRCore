module core
(
    // Основной интерфейс
    input       wire        clock,          // Тактовый генератор
    input       wire        locked,
    output      reg  [15:0] pc,             // Адрес PC
    output      reg  [15:0] address,        // Запрос в память
    input       wire [15:0] ir,             // Instruction Register
    input       wire [ 7:0] i_data,         // Считывание данных
    output      reg  [ 7:0] o_data,         // Запись данных
    output      reg         we              // Разрешение записи
);

`include "core.state.v"
`include "core.alu.v"

always @* begin

    pcload  = 0;                    // PC++
    wdata   = res;                  // Что будет записано в o_data
    alu     = 0;                    // 0=LDI
    reg_w   = 0;                    // =1 Запись в регистр reg_id
    sreg_w  = 0;                    // =1 Писать в sreg
    wren    = 0;                    // Запись в wdata из `res`
    stnext  = 0;                    // Счетчик `st`-ate

    casex (opcode)

        // [1T] LDI Rd, K
        16'b1110_xxxx_xxxx_xxxx: begin

            src     <= K;
            reg_w   <= 1;
            reg_id  <= rdi;

        end

    endcase

end

// ---------------------------------------------------------------------
always @(posedge clock) if (locked) begin

    if (st == 0) latch <= ir;

    // Обычно счетчики инкрементируются
    pc <= pc + 1;
    st <= stnext;

    if (reg_w)  r[ reg_id ] <= res;
    if (sreg_w) sreg <= flags;

    // Запись в память на следующем такте
    o_data <= wdata;
    we     <= wren;

    // Специальные системные регистры
    if (wren)
    case (address)

        // Системные регистры
        16'h005B: rampz         <= wdata; // Верхняя память ROM
        16'h005D: sp[ 7:0]      <= wdata; // SPL
        16'h005E: sp[15:8]      <= wdata; // SPH
        16'h005F: sreg          <= wdata; // SREG

        // Запись в регистры как в память
        default:  if (address < 16'h20) r[ address[4:0] ] <= wdata;

    endcase

end

endmodule
