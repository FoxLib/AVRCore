// ---------------------------------------------------------------------
// Проксирование памяти DIN
// ---------------------------------------------------------------------

reg [7:0] din;

always @* begin

    casex (address)

        // Регистры
        16'b0000_0000_000x_xxxx: din = r[ address[4:0] ];

        // Банки
        16'h0020: din = bank;
        16'h0021: din = 8'h00;

        // Клавиатура
        16'h0022: din = kb_ch;
        16'h0023: din = {7'h0, kb_hit};

        // Курсор
        16'h0024: din = cursor_x;
        16'h0025: din = cursor_y;

        // Таймер
        16'h0026: din = timer_ms[ 7:0];
        16'h0027: din = timer_ms[15:8];
        16'h002F: din = timer_ms[23:16];

        // SPI
        16'h0028: din = spi_din;
        16'h0029: din = spi_st;

        // Мышь
        16'h002A: din = mouse_x[7:0];
        16'h002B: din = mouse_y[7:0];
        16'h002C: din = {6'h0, mouse_cmd[1:0]};
        16'h002E: din = {7'h0, mouse_x[8]};

        // Процессор
        16'h005B: din = rampz;
        16'h005D: din = sp[ 7:0];
        16'h005E: din = sp[15:8];
        16'h005F: din = sreg;

        // SDRAM
        16'h0030: din = sdram_address[ 7:0];
        16'h0031: din = sdram_address[15:8];
        16'h0032: din = sdram_address[23:16];
        16'h0033: din = sdram_address[31:24];
        16'h0034: din = sdram_i_data;
        16'h0035: din = sdram_status;

        // Память
        default:  din = din_raw;

    endcase

end
