module adapter
(
    input   wire        CLOCK,
    output  reg  [3:0]  VGA_R,
    output  reg  [3:0]  VGA_G,
    output  reg  [3:0]  VGA_B,
    output  wire        VGA_HS,
    output  wire        VGA_VS,

    output  reg  [15:0] video_addr,
    input   wire [ 7:0] video_data
);

// ---------------------------------------------------------------------

// Тайминги для горизонтальной развертки
parameter hz_visible = 640;
parameter hz_front   = 16;
parameter hz_sync    = 96;
parameter hz_back    = 48;
parameter hz_whole   = 800;

// Тайминги для вертикальной развертки
parameter vt_visible = 400;
parameter vt_front   = 12;
parameter vt_sync    = 2;
parameter vt_back    = 35;
parameter vt_whole   = 449;

// ---------------------------------------------------------------------
assign VGA_HS = x  < (hz_back + hz_visible + hz_front); // NEG
assign VGA_VS = y >= (vt_back + vt_visible + vt_front); // POS
// ---------------------------------------------------------------------

wire        xmax = (x == hz_whole - 1);
wire        ymax = (y == vt_whole - 1);
reg  [10:0] x    = 0;
reg  [10:0] y    = 0;
wire [10:0] X    = x - hz_back; // X=[0..639]
wire [ 9:0] Y    = y - vt_back; // Y=[0..399]

// Бинарное представление
reg  [ 7:0] latch_data;

always @(posedge CLOCK) begin

    // Кадровая развертка
    x <= xmax ?         0 : x + 1;
    y <= xmax ? (ymax ? 0 : y + 1) : y;

    // Dummy-видеоадаптер на самом деле
    case (X[2:0])

        6: video_addr <= X[10:5] + Y[9:1]*40;
        7: latch_data <= video_data;

    endcase

    // Вывод окна видеоадаптера
    if (x >= hz_back && x < hz_visible + hz_back &&
        y >= vt_back && y < vt_visible + vt_back)
    begin
         {VGA_R, VGA_G, VGA_B} <= latch_data[ ~X[2:0] ] ? 12'hCCC : 12'h000;
    end
    else {VGA_R, VGA_G, VGA_B} <= 12'b0;

end

endmodule
