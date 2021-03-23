#include "avr.h"

// ---------------------------------------------------------------------
// ЧТЕНИЕ В ПАМЯТЬ И ПОРТЫ
// ---------------------------------------------------------------------

// Чтение из памяти
unsigned char APP::get(int addr) {

    addr &= 0xFFFF;

    // Банкинг памяти
    if (addr >= 0xF000) addr += 4096 * membank;

    unsigned char dv = 0;

    // Очистка бита в порту 00 при чтении
    switch (addr) {

        // Банк памяти
        case 0x20: return membank;
        case 0x21: return 0;

        // Клавиатура
        case 0x22: return port_keyb_xt;
        case 0x23: dv = port_keyb_hit; port_keyb_hit &= ~1; break;

        // Курсор
        case 0x24: dv = cursor_x; break;
        case 0x25: dv = cursor_y; break;

        // Таймер
        case 0x26: dv = timer & 0xff; break;
        case 0x27: dv = (timer >> 8) & 0xff; break;
        case 0x2F: dv = (timer >> 16) & 0xff; break;

        // SPI
        case 0x28: dv = spi_read_data(); break;
        case 0x29: dv = spi_read_status(); break;

        // Мышь
        case 0x2A: dv =  get_mouse_x();       break; // Lo Mouse
        case 0x2E: dv = (get_mouse_x() >> 8); break; // Hi Mouse
        case 0x2B: dv =  get_mouse_y();       break;
        case 0x2C: dv = mouse_cmd; break;

        // Видеорежим
        case 0x2D: return videomode;

        // SDRAM
        case 0x30: return  sdram_addr & 0xff;
        case 0x31: return (sdram_addr >> 8) & 0xff;
        case 0x32: return (sdram_addr >> 16) & 0xff;
        case 0x33: return (sdram_addr >> 24) & 0xff;
        case 0x34: return sdram_data[sdram_addr & 0x3ffffff]; // data
        case 0x35: return 0b00000010 | (sdram_ctl & 1); // ready=1, we=?

        // Остальная память
        default:   dv = sram[addr]; break;
    }

    return dv & 0xFF;
}

// Сохранение в память
void APP::put(int addr, unsigned char value) {

    addr &= 0xFFFF;

    // Учитывать банкинг
    if (addr >= 0xF000) addr += 4096 * membank;

    sram[addr] = value;

    // Запись во флаги
    if (addr == 0x5F) byte_to_flag(value);

    // Карта памяти
    if (addr == 0x20) { membank = value; }
    if (addr == 0x21) { /* membank */ }
    if (addr == 0x22) { port_keyb_xt  = value; }
    if (addr == 0x23) { port_keyb_hit = value; }

    // Обновление позиции курсора
    if (addr == 0x24) { cursor_x = value; update_text_xy(text_px, text_py); update_text_xy(cursor_x, cursor_y); }
    if (addr == 0x25) { cursor_y = value; update_text_xy(text_px, text_py); update_text_xy(cursor_x, cursor_y); }

    if (addr == 0x28) { spi_write_data(value); }
    if (addr == 0x29) { spi_write_cmd(value); }
    if (addr == 0x2D) { videomode = value; update_screen(); }

    // Управлению памятью
    if (addr == 0x30) sdram_addr = (sdram_addr & 0xFFFFFF00) | value;
    if (addr == 0x31) sdram_addr = (sdram_addr & 0xFFFF00FF) | (value << 8);
    if (addr == 0x32) sdram_addr = (sdram_addr & 0xFF00FFFF) | (value << 16);
    if (addr == 0x33) sdram_addr = (sdram_addr & 0x00FFFFFF) | (value << 24);
    if (addr == 0x34) sdram_data_byte = value;
    if (addr == 0x35) {

        if (value & 1)
            sdram_data[sdram_addr & 0x3ffffff] = sdram_data_byte;

        sdram_ctl = value;
    }

    if (addr == 0x36) intr_timer = value;

    // Нарисовать на холсте
    if (addr >= 0xC000) { update_byte_scr(addr); }
}

// ---------------------------------------------------------------------
// ПРОТОКОЛ SPI
// ---------------------------------------------------------------------

// Чтение последних данных
unsigned char APP::spi_read_data() {
    return spi_data;
}

// Чтение последнего статуса
unsigned char APP::spi_read_status() {
    return spi_st;
}

// Сохранение данных
void APP::spi_write_data(unsigned char data) {
    spi_data = data;
}

// Запуск команды
void APP::spi_write_cmd(unsigned char data) {

    if (data == 0) { spi_st &= ~2; /* reset chip, init */ }
    else if (data == 2) { /* enable chip */ }
    else if (data == 3) { /* disable chip */ }
    // data = 1
    else {

        //printf("%02x [%d]\n", spi_data, spi_status);

        switch (spi_status) {

            // IDLE
            case 0:

                // Чтение в 0-м дает последний ответ от команды
                if (spi_data == 0xFF) {

                    spi_data = spi_resp;
                    spi_resp = 0xFF;
                }
                // Запуск приема команды
                else if ((spi_data & 0xC0) == 0x40) {

                    spi_status  = 1;
                    spi_command = spi_data & 0x3F;
                    spi_arg     = 0;
                    spi_phase   = 0;
                }

                break;

            // COMMAND (прием)
            case 1: {

                if (spi_phase < 4)
                    spi_arg = (spi_arg << 8) | spi_data;

                // Окончание фазы команды
                if (spi_phase == 4) {

                    spi_phase = 0;
                    spi_crc   = spi_data;

                    /* Ответ зависит от команды */
                    switch (spi_command) {

                        /* CMDxx */
                        case 0:  spi_status = 0; spi_resp = 0x01; break;
                        case 8:  spi_status = 2; spi_resp = 0x00; break;
                        case 13: spi_status = 6; spi_resp = 0x00; break;    // STATUS
                        case 17: spi_status = 4; spi_lba  = spi_arg; break; // BLOCK SEARCH READ
                        case 24: spi_status = 5; spi_lba  = spi_arg; break; // BLOCK SEARCH WRITE
                        case 41: spi_status = 0; spi_resp = 0x00; break;    // READY=0
                        case 55: spi_status = 0; spi_resp = 0x01; break;    // ACMD=1
                        case 58: spi_status = 3; spi_resp = 0x00; break;    // CHECK=0
                        default: spi_status = 0; spi_resp = 0xFF; break;    // Unknown
                    }

                } else {
                    spi_phase++;
                }

                break;
            }

            // OCR Read (5 bytes)
            case 2: {

                if (spi_data == 0xFF) {

                    if (spi_phase == 4) {
                        spi_data = 0xAA;
                        spi_status = 0;
                    }
                    else spi_data = 0x00;

                    spi_phase++;
                }
                else {
                    printf("SPI Illegal Write #1"); exit(1);
                }

                break;
            }

            // Информация о SDHC поддержке
            case 3: {

                if (spi_data == 0xFF) {

                    if (spi_phase == 0) {
                        spi_data = 0x00;
                    } else if (spi_phase == 1) {
                        spi_data = 0xC0;
                    } else if (spi_phase < 4) {
                        spi_data = 0xFF;
                    } else {
                        spi_data = 0xFF;
                        spi_status = 0;
                    }

                    spi_phase++;

                } else {
                    printf("SPI Illegal Write #2"); exit(1);
                }

                break;
            }

            // Чтение с диска
            case 4: {

                if (spi_phase == 0) {

                    spi_data = 0x00;
                    spi_file = fopen("sd.img", "ab+");
                    if (spi_file == NULL) { printf("Error open file\n"); exit(0); }
                    fseek(spi_file, 512 * spi_lba, SEEK_SET);
                    (void) fread(spi_sector, 1, 512, spi_file);
                    fclose(spi_file);

                } else if (spi_phase == 1) {
                    spi_data = 0xFE;
                } else if (spi_phase < 514) {
                    spi_data = spi_sector[spi_phase - 2];
                }

                spi_phase++;
                if (spi_phase == 514) {

                    spi_status = 0;
                    spi_resp   = 0xFF;
                }

                break;
            }

            // Запись на диск
            case 5: {

                if (spi_phase == 0) {
                    spi_data = 0x00; // ACK

                } else if (spi_phase == 1) {

                    if (spi_data != 0xFE) {
                        printf("Illegal opcode (non FE)\n"); exit(1);
                    }

                } else if (spi_phase < 514) {
                    spi_sector[spi_phase - 2] = spi_data;

                } else if (spi_phase == 516) {
                    spi_data = 0x05; // ACK

                } else if (spi_phase < 520) {
                    spi_data = 0xFF;
                }

                spi_phase++;

                // Окончание программирования
                if (spi_phase == 520) {

                    spi_status = 0;
                    spi_resp   = 0x00;

                    // Запись новых данных на диск
                    spi_file = fopen("sd.img", "r+b");
                    if (spi_file == NULL) { printf("Error open file\n"); exit(0); }
                    fseek(spi_file, 512 * spi_lba, SEEK_SET);
                    (void) fwrite(spi_sector, 1, 512, spi_file);
                    fclose(spi_file);
                }

                break;
            }

            // STATUS [2 Byte 00 00]
            case 6: {

                if (spi_data == 0xFF) {

                    if (spi_phase == 1)
                        spi_status = 0;

                    spi_data = 0x00;
                    spi_phase++;
                }
                else {
                    printf("SPI Illegal Write #1"); exit(1);
                }
            }
        }
    }
}
