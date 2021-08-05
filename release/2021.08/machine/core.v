module core
(
    // Основной интерфейс
    input       wire        clock,          // Тактовый генератор
    input       wire        locked,
    output      reg  [15:0] pc,             // Адрес PC
    output      wire [15:0] address,        // Запрос в память
    input       wire [15:0] ir,             // Instruction Register
    input       wire [ 7:0] i_data,         // Считывание данных
    output      reg  [ 7:0] o_data,         // Запись данных
    output      reg         we              // Разрешение записи
);

// Либо указатель на адрес записи, либо на текущий курсор
assign address = we ? addr_w : cursor;

wire [15:0] __debug = r[0];

`include "core.state.v"
`include "core.alu.v"
`include "core.wb.v"

always @* begin

    // address -- не очищать
    pcstop  = (st != 0);    // =1 Не инкрементировать PC
    pcload  = 0;            // PC++
    wdata   = res;          // Что будет записано в o_data
    alu     = 0;            // 0=LDI
    reg_w   = 0;            // =1 Запись в регистр reg_id
    sreg_w  = 0;            // =1 Писать в sreg
    wren    = 0;            // Запись в wdata из `res`
    stnext  = 0;            // Счетчик `st`-ate
    reg_ww  = 0;            // Писать в X,Y,Z
    reg_ws  = 0;            // =1 Источник АЛУ; =0 Источник регистр `wb2`
    reg_wm  = 0;            // Запись в 1:0
    reg_idw = 0;            // Номер 16-битного регистра
    wb2     = 0;            // Данные для записи в X,Y,Z

    casex (opcode)

        // [1T] LDI Rd, K
        16'b1110_xxxx_xxxx_xxxx: begin

            src     = K;
            reg_w   = 1;
            reg_id  = rdi;

        end

        // [2T] LD Rd, (X|Y|Z)+
        // [1T] ST Rd, (X|Y|Z)+
        16'b1001_00xx_xxxx_1100, // X
        16'b1001_00xx_xxxx_1101, // X+
        16'b1001_00xx_xxxx_1110, // -X
        16'b1001_00xx_xxxx_1001, // Y+
        16'b1001_00xx_xxxx_1010, // -Y
        16'b1001_00xx_xxxx_0001, // Z+
        16'b1001_00xx_xxxx_0010: // -Z
        // Установка указателя на память
        begin

            wdata  = r[rd];
            wren   = opcode[9];

            // Выбор адреса
            case (opcode[3:0])

                4'b11_00: begin cursor = X;  end
                4'b11_01: begin cursor = X;  wb2 = Xp; reg_idw = 2'b01; reg_ww = 1; end
                4'b11_10: begin cursor = Xm; wb2 = Xm; reg_idw = 2'b01; reg_ww = 1; end
                4'b10_01: begin cursor = Y;  wb2 = Yp; reg_idw = 2'b10; reg_ww = 1; end
                4'b10_10: begin cursor = Ym; wb2 = Ym; reg_idw = 2'b10; reg_ww = 1; end
                4'b00_01: begin cursor = Z;  wb2 = Zp; reg_idw = 2'b11; reg_ww = 1; end
                4'b00_10: begin cursor = Zm; wb2 = Zm; reg_idw = 2'b11; reg_ww = 1; end

            endcase

            // Запись в регистр Rd (LD)
            reg_w   = ~opcode[9];
            alu     = 0;
            src     = din;
            reg_id  = rd;

        end

    endcase

end

endmodule
