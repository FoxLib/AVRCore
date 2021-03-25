`timescale 10ns / 1ns
module main;
// ---------------------------------------------------------------------
reg clk;
reg clk25;
always #0.5 clk = ~clk;
always #1.5 clk25 = ~clk25;

initial begin clk = 1; clk25 = 0; #2000 $finish; end
initial begin $dumpfile("main.vcd"); $dumpvars(0, main); end
initial begin $readmemh("program.hex", flashmem, 0); end
// ---------------------------------------------------------------------

reg [15:0] flashmem[4096];      // Доступно 8кб
reg [ 7:0] memory[65536];       // Доступны $0000-$17FF, $8000-$FFFF

always @(posedge clk) begin

    ir  <= flashmem[pc];
    din <= memory[address];
    if (w) memory[address] <= wb;

end

// ---------------------------------------------------------------------
wire [15:0] pc;
wire [15:0] address;
reg  [15:0] ir;
reg  [ 7:0] din;
wire [ 7:0] wb;
wire        w;

cpu AVRBench
(
    .clock      (clk25),
    .locked     (1'b1),
    .pc         (pc),
    .ir         (ir),
    .address    (address),
    .din_raw    (din),
    .wb         (wb),
    .w          (w)
);

endmodule
