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
    input  wire [7:0]   data_o_grph,    // Данные из GRPH-памяти
    input  wire [7:0]   data_o_32kb,    // Данные из 32KB-памяти
    output reg          data_w_sram,    // Разрешение записи в память SRAM
    output reg          data_w_text,    // Разрешение записи в память ROM
    output reg          data_w_grph,    // Разрешение записи в память GRPH
    output reg          data_w_32kb,    // Разрешение записи в память 32KB

    // Курсор VGA
    output reg  [7:0]   cursor_x,
    output reg  [7:0]   cursor_y,

    // Клавиатура
    input wire  [7:0]   ps2_data,
    input wire          ps2_hit,

    // Видеорежим
    output reg  [7:0]   videomode,

    // SDRAM
    output reg [31:0]   sdram_address,
    output reg [ 7:0]   sdram_i_data,
    input wire [ 7:0]   sdram_o_data,
    output reg          sdram_we,
    input wire          sdram_ready,

    // SD SPI
    output reg  [1:0]   sd_cmd,
    input  wire [7:0]   sd_din,
    output reg  [7:0]   sd_out,
    output reg          sd_signal,
    input  wire         sd_busy,
    input  wire         sd_timeout
);

reg        keyb_up;
reg  [7:0] keyb_data;
wire [7:0] keyb_ascii;
reg        keyb_hit_i;
reg        keyb_hit_o;
reg        keyb_shift;

// Маршрутизация памяти
always @* begin

    data_i      = data_o_sram;
    data_w_sram = wren;
    data_w_text = 1'b0;
    data_w_grph = 1'b0;
    data_w_32kb = 1'b0;

    // Запись в банки памяти
    if (address >= 16'hF000)
    casex (bank)

        8'b00000000: begin /* верхняя страница */ end
        8'b0000001x: begin data_w_text = wren; data_w_sram = 1'b0; data_i = data_o_text; end
        8'b001xxxxx: begin data_w_grph = wren; data_w_sram = 1'b0; data_i = data_o_grph; end
        8'b01000xxx: begin data_w_32kb = wren; data_w_sram = 1'b0; data_i = data_o_32kb; end
        default:     begin data_w_sram = 1'b0; data_i = 8'hFF; end

    endcase
    // Чтение из портов
    else case (address)

        /* BANK   */ 16'h20: data_i = bank;
        /* KEYB   */ 16'h21: data_i = keyb_data;
        /* STATUS */ 16'h22: data_i = {
            /* 7 RW DRAM-WE  */ sdram_we,
            /* 6 R  DRAM-BSY */ ~sdram_ready,
            /* 5 R  SD-BSY   */ sd_busy,
            /* 4 R  SD-TIME  */ sd_timeout,
            /* 3 -           */ 1'b0,
            /* 2 -           */ 1'b0,
            /* 1 -           */ 1'b0,
            /* 0 R  KBD-CNT  */ keyb_hit_i ^ keyb_hit_o
        };
        /* CURSX  */ 16'h2C: data_i = cursor_x;
        /* CURSY  */ 16'h2D: data_i = cursor_y;
        /* SDRAM0 */ 16'h30: data_i = sdram_address[ 7: 0];
        /* SDRAM1 */ 16'h31: data_i = sdram_address[15: 8];
        /* SDRAM2 */ 16'h32: data_i = sdram_address[23:16];
        /* SDRAM3 */ 16'h33: data_i = sdram_address[31:24];
        /* SDRAMD */ 16'h34: data_i = sdram_o_data;
        /* VIDEO  */ 16'h38: data_i = videomode;
        /* SD-DAT */ 16'h39: data_i = sd_din;
        /* SD-CMD */ 16'h3A: data_i = {sd_signal, 5'b00000, sd_cmd[1:0]};

    endcase

end

// Регистрация записи и чтения в порты
always @(negedge clock) begin

    if (wren)
    case (address)

        16'h20: bank <= data_o;

        // Запись защелки
        16'h22: begin

            // При записи в STATUS[4] выполнить сброс KBHIT -> 0
            if (data_o[0] && keyb_hit_i != keyb_hit_o)
                keyb_hit_i <= ~keyb_hit_i;

            // Запись в DRAM
            sdram_we <= data_o[7];

        end
        16'h2C: cursor_x <= data_o;
        16'h2D: cursor_y <= data_o;
        16'h30: sdram_address[ 7: 0] <= data_o;
        16'h31: sdram_address[15: 8] <= data_o;
        16'h32: sdram_address[23:16] <= data_o;
        16'h33: sdram_address[31:24] <= data_o;
        16'h34: sdram_i_data <= data_o;
        16'h38: videomode    <= data_o;
        16'h39: sd_out       <= data_o;

        // Защелка + команда на SPI
        16'h3A: {sd_signal, sd_cmd} <= {data_o[7], data_o[1:0]};

    endcase

end

// Прием символа с клавиатуры
always @(posedge clock50) begin

    if (ps2_hit) begin

        // Клавиша была отпущена
        if (ps2_data == 8'hF0) keyb_up <= 1'b1;

        // Все остальные клавиши
        else begin

            // Левый или правый SHIFT
            if (ps2_data == 8'h12 || ps2_data == 8'h59)
                keyb_shift <= ~keyb_up;

            keyb_data  <= {keyb_up, keyb_ascii[6:0]};
            keyb_up    <= 1'b0;

            // Установить единицу на STATUS[4]
            if (keyb_hit_o == keyb_hit_i)
                keyb_hit_o <= ~keyb_hit_o;

        end

    end

end

// Конвертер AT -> ASCII
at2ascii UnitAT2ASCII
(
    .at     (ps2_data),
    .ascii  (keyb_ascii),
    .shift  (keyb_shift)
);

endmodule

`include "at2ascii.v"
