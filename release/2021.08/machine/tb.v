`timescale 10ns / 1ns
module tb;

reg clock;
reg clock_25;
reg clock_50;

always #0.5 clock    = ~clock;
always #1.0 clock_50 = ~clock_50;
always #2.5 clock_25 = ~clock_25;

initial begin clock = 1; clock_25 = 1; clock_50 = 1; #2000 $finish; end
initial begin $dumpfile("tb.vcd"); $dumpvars(0, tb); end

// ---------------------------------------------------------------------
reg  [15:0] progmem[65536];
reg  [ 7:0] memdata[65536];
// ---------------------------------------------------------------------
initial begin $readmemh("tb.hex", progmem, 0); end
// ---------------------------------------------------------------------
wire [15:0] pc;
wire [15:0] address;
reg  [15:0] ir;
reg  [ 7:0] i_data;
wire [ 7:0] o_data;
wire        we;
// ---------------------------------------------------------------------

// Контроллер блочной памяти
always @(posedge clock) begin

    i_data <= memdata[ address ];
    ir     <= progmem[ pc      ];

    if (we) memdata[ address ] <= o_data;

end

// ---------------------------------------------------------------------
core CoreUnit
(
    .clock      (clock_25),
    .pc         (pc),
    .ir         (ir),
    .address    (address),
    .i_data     (i_data),
    .o_data     (o_data),
    .we         (we)
);

endmodule
