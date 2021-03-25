// Запись в регистры
always @(negedge clock) begin

    // Блокировка процессора на время I/O
    if (!locked) begin

        if (reg_w) r[ reg_id ] <= alu_res;
        if (sreg_w) sreg <= alu_sreg;

        // Икремент или декремент
        case (sp_mth)

            `SPDEC: sp <= sp - 1;
            `SPINC: sp <= sp + 1;

        endcase

        // Автоинкремент или декремент X,Y,Z
        if (reg_ww) begin

            case (reg_idw)

                0: {r[25], r[24]} <= reg_ws ? resw : wb2; // W
                1: {r[27], r[26]} <= reg_ws ? resw : wb2; // X
                2: {r[29], r[28]} <= reg_ws ? resw : wb2; // Y
                3: {r[31], r[30]} <= reg_ws ? resw : wb2; // Z

            endcase

        end

        // Запись в порты или регистры
        if (w) begin

            case (address)

                // Управление памятью, курсором, SPI
                16'h0020: bank     <= wb; // memory.bank (4kb) $F000
                16'h0024: cursor_x <= wb; // text.cursor.x
                16'h0025: cursor_y <= wb; // text.cursor.y
                16'h0028: spi_out  <= wb; // spi.data

                // Запуск триггера активации команды SPI
                16'h0029: begin

                    spi_cmd  <= wb[1:0];
                    spi_sent <= 1'b1;
                    locked   <= 1'b1;

                end

                16'h002D: vmode <= wb; // Видеорежим

                // SDRAM
                16'h0030: sdram_address[ 7:0]  <= wb;
                16'h0031: sdram_address[15:8]  <= wb;
                16'h0032: sdram_address[23:16] <= wb;
                16'h0033: sdram_address[31:24] <= wb;
                16'h0034: sdram_o_data         <= wb;
                16'h0035: sdram_control        <= wb;

                // Таймер прерывания
                16'h0036: intr_maxtime <= wb;

                // Системные регистры
                16'h005B: rampz     <= wb; // Верхняя память ROM
                16'h005D: sp[ 7:0]  <= wb; // SPL
                16'h005E: sp[15:8]  <= wb; // SPH
                16'h005F: sreg      <= wb; // SREG

                // Запись в регистры как в память
                default:

                    if (address < 16'h20) r[ address[4:0] ] <= wb;

            endcase

        end

        // Тест на чтение из порта
        if (aread) begin

            // Сбросить бит 0 в порту 23h. Реагировать только если бит был реально прочитан!
            if (address == 16'h0023 && kbit) kb_trigger <= kb_trigger ^ kb_hit;

        end

    end

    // -----------------------------------------------------------------
    // Разблокировка процессора при BUSY=0
    if (spi_proc && spi_st[0] == 1'b0) begin
        locked   <= 0;
        spi_proc <= 0;
    end

    // Сброс SENT, если устройство SPI начало работу
    if (spi_sent && spi_st[0]) begin
        spi_sent <= 1'b0;
        spi_proc <= 1'b1;
    end
    // -----------------------------------------------------------------

end
