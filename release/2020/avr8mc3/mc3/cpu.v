module cpu
(
    input  wire         clock,
    input  wire         locked,

    // Программная память
    output reg  [15:0]  pc,          // Программный счетчик
    input  wire [15:0]  ir,          // Инструкция из памяти

    // Оперативная память
    output reg  [15:0]  address,     // Указатель на память RAM (sram)
    input  wire [ 7:0]  din_raw,     // memory[ address ]
    output reg  [ 7:0]  wb,          // Запись в память по address
    output reg          w            // Разрешение записи в память
);

`include "inc_declare.v"
`include "inc_timer.v"
`include "inc_wbreg.v"

// Исполнительное устройство
always @(posedge clock)
if (locked) begin

    w      <= 1'b0;
    aread  <= 1'b0;
    reg_w  <= 1'b0;
    sreg_w <= 1'b0;
    sp_mth <= 1'b0; // Ничего не делать с SP
    reg_ww <= 1'b0; // Ничего не делать с X,Y,Z
    reg_ws <= 1'b0; // Источник регистр wb2

    if (tstate == 0) latch <= ir;

    // Код пропуска инструкции
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

    // Прерывание
    `include "inc_interrupt.v"

    // Исполнение опкодов
    else casex (opcode)
    `include "cpu_execute.v"
    endcase

end

endmodule
