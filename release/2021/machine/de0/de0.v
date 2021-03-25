module de0(

    // Reset
    input              RESET_N,

    // Clocks
    input              CLOCK_50,
    input              CLOCK2_50,
    input              CLOCK3_50,
    inout              CLOCK4_50,

    // DRAM
    output             DRAM_CKE,
    output             DRAM_CLK,
    output      [1:0]  DRAM_BA,
    output      [12:0] DRAM_ADDR,
    inout       [15:0] DRAM_DQ,
    output             DRAM_CAS_N,
    output             DRAM_RAS_N,
    output             DRAM_WE_N,
    output             DRAM_CS_N,
    output             DRAM_LDQM,
    output             DRAM_UDQM,

    // GPIO
    inout       [35:0] GPIO_0,
    inout       [35:0] GPIO_1,

    // 7-Segment LED
    output      [6:0]  HEX0,
    output      [6:0]  HEX1,
    output      [6:0]  HEX2,
    output      [6:0]  HEX3,
    output      [6:0]  HEX4,
    output      [6:0]  HEX5,

    // Keys
    input       [3:0]  KEY,

    // LED
    output      [9:0]  LEDR,

    // PS/2
    inout              PS2_CLK,
    inout              PS2_DAT,
    inout              PS2_CLK2,
    inout              PS2_DAT2,

    // SD-Card
    output             SD_CLK,
    inout              SD_CMD,
    inout       [3:0]  SD_DATA,

    // Switch
    input       [9:0]  SW,

    // VGA
    output      [3:0]  VGA_R,
    output      [3:0]  VGA_G,
    output      [3:0]  VGA_B,
    output             VGA_HS,
    output             VGA_VS
);

// Z-state
assign DRAM_DQ = 16'hzzzz;
assign GPIO_0  = 36'hzzzzzzzz;
assign GPIO_1  = 36'hzzzzzzzz;

// LED OFF
assign HEX0 = 7'b1111111;
assign HEX1 = 7'b1111111;
assign HEX2 = 7'b1111111;
assign HEX3 = 7'b1111111;
assign HEX4 = 7'b1111111;
assign HEX5 = 7'b1111111;

// Генерация частот
wire locked;
wire clock_25;
wire clock_100;
wire clock_cpu = clock_25 & locked;

de0pll unit_pll
(
    .clkin     (CLOCK_50),
    .m25       (clock_25),
    .m100      (clock),
    .locked    (locked)
);

// ---------------------------------------------------------------------
// Объявление процессора
// ---------------------------------------------------------------------

wire [15:0] pc;         // Программный счетчик
reg  [15:0] ir;         // Регистр инструкции (текущий опкод)
wire [15:0] address;    // Адрес в памяти SRAM
wire [ 7:0] data_i;     // Прочтенные данные из SRAM
wire [ 7:0] data_o;     // Данные для записи в SRAM
wire        wren;       // Разрешение на запись в память

avrcpu ModuleCPU
(
    // Тактовый генератор
    .clock      (clock_cpu),

    // Программная память
    .pc         (pc),           // Программный счетчик
    .ir         (ir),           // Инструкция из памяти

    // Оперативная память
    .address    (address),      // Указатель на память RAM (sram)
    .data_i     (data_i),       // memory[ address ]
    .data_o     (data_o),       // Запись в память по address
    .wren       (wren)          // Разрешение записи в память
);

// ---------------------------------------------------------------------
// Контроллер памяти
// ---------------------------------------------------------------------

wire [7:0]  bank;
wire        data_w_sram;
wire        data_w_text;
wire [7:0]  data_o_sram;
wire [7:0]  data_o_text;

memctrl UnitMemoryController(

    .clock          (clock_cpu),
    .address        (address),
    .wren           (wren),

    .bank           (bank),
    .data_i         (data_i),
    .data_o         (data_o),
    .data_o_sram    (data_o_sram),
    .data_o_text    (data_o_text),
    .data_w_sram    (data_w_sram),
    .data_w_text    (data_w_text),

    .cursor_x       (cursor_x),
    .cursor_y       (cursor_y),
);

// ---------------------------------------------------------------------
// Модули внутрисхемной памяти
// ---------------------------------------------------------------------

// WORD 32k (64k) Память программ
memflash UnitMemFlash
(
    .clock     (clock),
    .address_a (pc[14:0]),
    .q_a       (ir),
);

// BYTE 64k Общая оперативная память
memsram UnitMemSram
(
    .clock     (clock),
    .address_a (address),
    .q_a       (data_o_sram),
    .data_a    (data_o),
    .wren_a    (data_w_sram),
);

// BYTE 8k Видеопамять текстового режима
memtext UnitMemtext
(
    .clock     (clock),
    .address_a ({bank[0], address[11:0]}),
    .address_b (text_address),
    .q_a       (data_o_text),
    .q_b       (text_data),
    .data_a    (data_o),
    .wren_a    (data_w_text),
);

// 128k Видеопамять графического режима
// 32k  Дополнительная память

// ---------------------------------------------------------------------
// Видеоадаптер
// ---------------------------------------------------------------------

wire [12:0] text_address;
wire [ 7:0] text_data;
wire [ 7:0] cursor_x;
wire [ 7:0] cursor_y;

vga unit_vga
(
    .CLOCK  (clock_25),
    .VGA_R  (VGA_R),
    .VGA_G  (VGA_G),
    .VGA_B  (VGA_B),
    .VGA_HS (VGA_HS),
    .VGA_VS (VGA_VS),

    // Доступ к памяти
    .text_address (text_address),
    .text_data    (text_data),

    // Курсор
    .cursor_x (cursor_x),
    .cursor_y (cursor_y),
);

endmodule

`include "../avrcpu.v"
`include "../vga.v"
`include "../memctrl.v"
