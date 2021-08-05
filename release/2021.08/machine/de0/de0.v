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
wire clock_50;
wire clock_100;

// ---------------------------------------------------------------------
wire [15:0] pc;
wire [15:0] address;
wire [15:0] ir;
wire [ 7:0] i_data;
wire [ 7:0] o_data;
wire        we;

// ---------------------------------------------------------------------
de0pll PLLUnit
(
    .clkin     (CLOCK_50),
    .m25       (clock_25),
    .m50       (clock_50),
    .m100      (clock_100),
    .locked    (locked)
);

// ---------------------------------------------------------------------
adapter AdapterUnit
(
    .CLOCK      (0 && clock_25),
    .VGA_R      (VGA_R),
    .VGA_G      (VGA_G),
    .VGA_B      (VGA_B),
    .VGA_HS     (VGA_HS),
    .VGA_VS     (VGA_VS)
);

// Процессор
// ---------------------------------------------------------------------

core CoreUnit
(
    .clock      (clock_25),
    .locked     (locked),
    .pc         (pc),
    .ir         (ir),
    .address    (address),
    .i_data     (i_data),
    .o_data     (o_data),
    .we         (we)
);

// Модули памяти
// ---------------------------------------------------------------------

// Память программ
program UnitProgram
(
    .clock     (clock_100),
    .address_a (pc[9:0]),
    .q_a       (ir)
);

// 64k память
memory UnitMemory
(
    .clock     (clock_100),
    .address_a (address),
    .q_a       (i_data),
    .data_a    (o_data),
    .wren_a    (we),
    // Видеопамять
    // .address_b (address_b),
    // .q_b       (q_b),
);

endmodule

`include "../core.v"
`include "../adapter.v"
