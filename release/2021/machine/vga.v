module vga
(
    input   wire        CLOCK,
    output  reg  [3:0]  VGA_R,
    output  reg  [3:0]  VGA_G,
    output  reg  [3:0]  VGA_B,
    output  wire        VGA_HS,
    output  wire        VGA_VS,

    // Видеорежим
    input   wire [ 7:0] videomode,

    // Текущее положение курсора
    input   wire [ 7:0] cursor_x,
    input   wire [ 7:0] cursor_y,

    // Доступ к текстовой видеопамяти 8k
    output  reg  [12:0] text_address,
    input   wire [ 7:0] text_data,
    output  reg  [16:0] grph_address,
    input   wire [ 7:0] grph_data
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
wire [10:0] X    = x - hz_back + 8; // X=[0..639]
wire [10:0] XG   = x - hz_back + 2;
wire [ 9:0] Y    = y - vt_back;     // Y=[0..399]

// Вычисление позиции курсора, его наличие.
wire        cursor   = ((cursor_x + 1 == X[9:3]) && (cursor_y == Y[9:4])) && (Y[3:0] >= 14);
wire [15:0] cursor_g = 320*Y[9:1] + XG[10:1]; // 320x200
wire [16:0] cursor_h = 320*Y      + XG[10:1]; // 640x400

// Рендеринг шрифта
wire        cubit = font_data[ 3'h7 ^ X[2:0] ];
wire [11:0] color = cubit ^ (cursor & flash) ? cl_fore : cl_back;

// Символ для рисования
reg         flash;
reg  [ 7:0] text_char;
reg  [ 7:0] text_attr;
reg  [11:0] cl_fore_;  reg  [11:0] cl_fore;
reg  [11:0] cl_back_;  reg  [11:0] cl_back;
reg  [ 7:0] color_320;
reg  [ 7:0] font_data;

always @(posedge CLOCK) begin

    // Кадровая развертка
    x <= xmax ?         0 : x + 1;
    y <= xmax ? (ymax ? 0 : y + 1) : y;

    // Текстовый режим
    case (X[2:0])

        // Запрос на символ
        3'b000: begin text_address   <= 2*(X[9:3] + 80*Y[9:4]); end

        // Прочитать символ, запрос на атрибут
        3'b001: begin text_address   <= {text_address[11:1], 1'b1};
                      text_char      <= text_data; end

        // Прочитать атрибут, запрос на FORECOLOR
        3'b010: begin text_address   <= {12'hFA0 + 2*text_data[3:0]};
                      text_attr      <= text_data; end

        // Прочитать FC[7:0], добавить +1
        3'b011: begin text_address   <= {text_address[11:1], 1'b1};
                      cl_fore_[7:0]  <= text_data; end

        // Прочитать FC[11:8], запрос BACKCOLOR
        3'b100: begin text_address   <= {12'hFA0 + 2*text_attr[7:4]};
                      cl_fore_[11:8] <= text_data[3:0]; end

        // Прочитать BC[7:0], добавить +1
        3'b101: begin text_address   <= {text_address[11:1], 1'b1};
                      cl_back_[7:0]  <= text_data; end

        // Прочитать BC[11:8], запрос знакоместа
        3'b110: begin text_address   <= {1'b1, text_char, Y[3:0]};
                      cl_back_[11:8] <= text_data[3:0]; end

        // Прочитать знакоместо, триггер для отрисовки
        3'b111: begin font_data <= text_data;
                      cl_fore   <= cl_fore_;
                      cl_back   <= cl_back_; end

    endcase

    // Графические режимы
    case (videomode)

        // 640x400x16
        // 1: case (XG[0]) 0: grph_address <= ...; 1: color_640 <= grph_data; endcase

        // 320x200x256
        2, 3: case (XG[0])

            0: begin grph_address <= {videomode[0], cursor_g}; end
            1: begin color_320    <= grph_data; end

        endcase

    endcase

    // Вывод окна видеоадаптера
    if (x >= hz_back && x < hz_visible + hz_back &&
        y >= vt_back && y < vt_visible + vt_back)
    begin

        case (videomode)
        2, 3:    {VGA_R, VGA_G, VGA_B} <=
        {   // 3:3:2
            color_320[7:5],color_320[5],             // 3
            color_320[4:2],color_320[2],             // 3
            color_320[1:0],color_320[0],color_320[0] // 2
        };

        default: {VGA_R, VGA_G, VGA_B} <= color;
        endcase

    end
    else {VGA_R, VGA_G, VGA_B} <= 12'b0;

end

// ---------------------------------------------------------------------
// Мерцающий курсор
// ---------------------------------------------------------------------
reg [23:0]  clock;
wire        clock_tick = (clock == 6250000);

// Таймер, 0.5 с
always @(posedge CLOCK) begin

    flash <= clock_tick ? ~flash : flash;
    clock <= clock_tick ? 1'b0   : clock + 1;

end

endmodule
