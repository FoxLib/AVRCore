module memctrl(

    input  wire         clock,
    input  wire         clock50,
    input  wire [15:0]  address,
    input  wire         wren,

    // Внутрисхемная память
    output reg  [7:0]   bank,
    output reg  [7:0]   data_i,         // Выходящие данные в процессор AVR
    input  wire [7:0]   data_o,         // Данные от процессора
    input  wire [7:0]   data_o_sram,    // Данные из SRAM-памяти
    input  wire [7:0]   data_o_text,    // Данные из TEXT-памяти
    output reg          data_w_sram,    // Разрешение записи в память SRAM
    output reg          data_w_text,    // Разрешение записи в память ROM

    // Курсор VGA
    output reg  [7:0]   cursor_x,
    output reg  [7:0]   cursor_y,

    // Клавиатура
    input wire  [7:0]   ps2_data,
    input wire          ps2_hit
);

reg        keyb_up;
reg  [7:0] keyb_data;
wire [7:0] keyb_ascii;
reg        keyb_latch;

// Маршрутизация памяти
always @* begin

    data_i      = data_o_sram;
    data_w_sram = wren;
    data_w_text = 1'b0;

    // Запись в банки памяти
    if (address >= 16'hF000)
    casex (bank)

        8'b0000001x: begin data_w_text = wren; data_w_sram = 1'b0; data_i = data_o_text; end
        default:     begin data_w_sram = 1'b0; data_i = 8'hFF; end

    endcase
    // Чтение из портов
    else case (address)

        16'h20: data_i = bank;
        16'h21: data_i = keyb_data;             // KEYB
        16'h22: data_i = {/*0*/keyb_latch};          // STATUS
        16'h2C: data_i = cursor_x;
        16'h2D: data_i = cursor_y;

    endcase

end

// Регистрация записи и чтения в порты
always @(negedge clock) begin

    if (wren)
    case (address)

        16'h20: bank     <= data_o;
        // 16'h22: // SDRAM WE
        16'h2C: cursor_x <= data_o;
        16'h2D: cursor_y <= data_o;
        // 16'h38: // VIDEOMODE

    endcase

end

// Прием символа с клавиатуры [пока что в RAW-виде в данный момент]
always @(posedge clock50) begin

    if (ps2_hit) begin

        // Клавиша отпущена
        if (ps2_data == 8'hF0) keyb_up <= 1'b1;
        else begin

            keyb_data  <= {keyb_up, keyb_ascii[6:0]};
            keyb_latch <= ~keyb_latch;
            keyb_up    <= 1'b0;

        end

    end

end

// Конвертер AT -> ASCII
at2ascii UnitAT2ASCII
(
    .at (ps2_data),
    .xt (keyb_ascii)
);

endmodule

`include "at2ascii.v"
