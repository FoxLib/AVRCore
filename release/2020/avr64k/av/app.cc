#include "main.h"

// Конструктор
APP::APP() {

    // Инициализация
    pc              = 0x0000;
    ds_start        = 0;
    ds_cursor       = 0;
    ds_debugger     = 1;
    ds_tab          = 0;
    ds_dump_cursor  = 0;
    ds_dump_start   = 0;

    cpu_halt        = 1;
    require_halt    = 0;
    instr_counter   = 0;
    framecycle      = 0;
    ds_brk_cnt      = 0;

    port_kb_cnt     = 0;
    mouse_cmd       = 0;
    intr_timer      = 0;
    last_timer      = 0;

    cpu_model       = ATTINY85;
    max_flash       = 0x1FFF;
    bank            = 0;

    // Видеорежимы
    videom          = 0;
    flash           = 0;
    flash_cnt       = 0;
    cursor_x        = 0;
    cursor_y        = 0;

    // SPI
    spi_st          = 2;         // SPI_TIMEOUT set
    spi_cmd         = 0;
    spi_data        = 0;
    spi_status      = 0;
    spi_command     = 0;
    spi_phase       = 0;
    spi_latch       = 0;

    dram_address    = 0;
    count_per_frame = 200000;    // 10,0 mHz процессор

    // Общая память
    dynamic_ram     = (uint8_t*) malloc(64*1024*1024);
}

APP::~APP() {

    free(dynamic_ram);
}

// Загрузка файла в память
void APP::loadfile(const char* fn) {

    FILE* fp = fopen(fn, "rb");
    if (fp) {

        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(program, 1, size, fp);
        fclose(fp);

        // Attiny85
        if ((program[1] & 0xF0) == 0xC0) {

            cpu_model = ATTINY85;
            max_flash = 0x1FFF;

        } else if ((program[1] & 0xFF) == 0x94) {
            cpu_model = ATMEGA328;
            max_flash = 0x7FFF;
        }

    } else {

        printf("Указанный файл не был найден\n");
        exit(1);
    }
}

// Загрузка конфигурации
void APP::config() {

    clock_mhz       = 10;
    clock_video     = 50;
    config_width    = 1280;
    config_height   = 800;

    FILE* fp = fopen("config.ini", "r");

    if (fp) {

        fscanf(fp, "%d", & clock_mhz);
        fscanf(fp, "%d", & clock_video);

        // Инструкции за кадр
        count_per_frame = (clock_mhz * 1000000) / clock_video;

        fclose(fp);
    }
}

// Главный обработчик приложения
void APP::infinite() {

    int k, i, keyid, mk;
    int jump_dump = (height - 16*18);
    int jump_dis  = (height - 32) / 8;

    while (1) {

        int redraw = 0;
        while (SDL_PollEvent(& event)) {

            switch (event.type) {

                // Если нажато на крестик, то приложение будет закрыто
                case SDL_QUIT: return;

                // Движение мыши
                case SDL_MOUSEMOTION: {

                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    break;
                }

                // Нажата мышь
                case SDL_MOUSEBUTTONDOWN: {

                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    mk      = event.motion.state;

                    if (mk == SDL_BUTTON_LEFT)       mouse_cmd |= 1;
                    else if (mk == SDL_BUTTON_RIGHT) mouse_cmd |= 2;

                    // Установка курсора мыши куда следует
                    if (mouse_x >= 8 && mouse_x < 54*8 && mouse_y >= 16 && mouse_y < 44*16 && ds_debugger) {

                        ds_cursor = ds_addresses[ mouse_y>>4 ];
                        swi_brk();
                        ds_update();
                    }

                    break;
                }

                // Мышь отпущена
                case SDL_MOUSEBUTTONUP: {

                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    mk      = event.motion.state;

                    if (mk == SDL_BUTTON_LEFT)       mouse_cmd &= ~1;
                    else if (mk == SDL_BUTTON_RIGHT) mouse_cmd &= ~2;

                    break;
                }

                // Нажата какая-то клавиша
                case SDL_KEYDOWN: {

                    keyid = get_key_ascii(event);

                    // Режим обычной работы процессора
                    if (!ds_debugger && !cpu_halt) {

                        if (keyid > 0) {

                            port_kb_cnt++;
                            port_keyb_xt = keyid;
                        }
                    }

                    // F7 Процессор должен быть остановлен для некоторых команд
                    // F3 Вызов прерывания
                    if (cpu_halt) {

                        // Выполнить шаг
                        if (keyid == key_F7) { step(); ds_cursor = pc; }

                        // Вызов прерывания
                        if (keyid == key_F8) { interruptcall(); ds_cursor = pc; }

                        // Если запущена отладка, обновить
                        if (ds_debugger) ds_update();
                    }

                    // F2 Остановка
                    if (keyid == key_F2 && ds_debugger) {

                        swi_brk();
                        ds_update();
                    }

                    // F4 Посмотреть окно (рабочее) или отладчик
                    if (keyid == key_F4) {

                        ds_debugger ^= 1;
                        update_screen();
                    }

                    // F9 Запуск программы (либо остановка)
                    if (keyid == key_F9) {

                        // Процессор остановлен, запустить его
                        if (cpu_halt) {

                            cpu_halt = 0;
                            ds_debugger = 0;
                            update_screen();
                        }
                        // Послать сигнал остановки
                        else {
                            require_halt = 1;
                        }
                    }

                    // +/- на NumPad
                    if (keyid == -82) { count_per_frame /= 1.5; printf("cycles: %d\n", count_per_frame); }
                    if (keyid == -86) { count_per_frame *= 1.5; printf("cycles: %d\n", count_per_frame); }

                    // TAB
                    if (keyid == key_TAB) {

                        if (ds_debugger) {

                            // 0 - Отладчик
                            // 1 - Данные
                            ds_tab = (ds_tab + 1) % 2;
                            ds_update();
                        }
                    }

                    // PgDn
                    if (keyid == key_PGDN && ds_debugger) {

                        if (ds_tab == 1) {
                            ds_dump_cursor += jump_dump;
                            ds_dump_start  += jump_dump;
                        }
                        else if (ds_tab == 0) {
                            ds_start  += jump_dis;
                            ds_cursor += jump_dis;
                        }

                        ds_update();
                    }

                    // PgUp
                    if (keyid == key_PGUP && ds_debugger) {

                        if (ds_tab == 1) {
                            ds_dump_cursor -= jump_dump;
                            ds_dump_start  -= jump_dump;
                        }
                        else if (ds_tab == 0) {

                            ds_start  -= jump_dis;
                            ds_cursor -= jump_dis;

                            if (ds_start < 0) ds_start = 0;
                            if (ds_cursor < 0) ds_cursor = 0;
                        }

                        ds_update();
                    }

                    // KeyDown
                    if (keyid == key_DN && ds_debugger) {

                        if (ds_tab == 1) {

                            ds_dump_cursor += 0x010;
                            if (ds_dump_cursor >= ds_dump_start + 0x1B0)
                                ds_dump_start = ds_dump_cursor;
                        }
                        else if (ds_tab == 0) {

                            ds_cursor += 2;
                        }

                        ds_update();
                    }

                    // KeyUp
                    if (keyid == key_UP && ds_debugger) {

                        if (ds_tab == 1) {

                            ds_dump_cursor -= 0x010;
                            if (ds_dump_cursor < 0)
                                ds_dump_cursor = 0;

                            if (ds_dump_cursor < ds_dump_start)
                                ds_dump_start = ds_dump_cursor;
                        }
                        else if (ds_tab == 0) {

                            ds_cursor -= 2;
                            if (ds_cursor < 0) ds_cursor = 0;
                        }

                        ds_update();
                    }

                    // Горячие клавиши
                    if (ds_debugger && ds_tab == 1) {

                        if (keyid == 'R') ds_dump_cursor = ds_dump_start = 0x0000;
                        if (keyid == 'V') ds_dump_cursor = ds_dump_start = 0xc000;

                        ds_update();
                    }

                    // Нажатие горящей путевки в режиме ds_tab
                    if (ds_debugger && ds_tab == 0) {

                        if (keyid == 'R') { pc = ds_start = ds_cursor = 0; instr_counter = 0; ds_update(); }
                    }

                    break;
                }

                // Отпущена клавиша
                case SDL_KEYUP: {

                    keyid = get_key_ascii(event);

                    // Клавиша отпускается
                    if (!ds_debugger && !cpu_halt) {

                        if (keyid > 0) {

                            port_kb_cnt++;
                            port_keyb_xt = 0x80 | keyid;
                        }
                    }

                    break;
                }

                // Вызывается по таймеру
                case SDL_USEREVENT: redraw = 1; timer = (timer + 20); break;
            }
        }

        // Обнаружено событие обновления экрана
        if (redraw) {

            // Если запрошена остановка
            if (require_halt) {

                cpu_halt    = 1;
                ds_debugger = 1;
                ds_cursor   = pc;
                update_screen();
            }

            // Исполнить код, если процессор запущен
            if (cpu_halt == 0) {

                // Выполнить инструкции
                while (framecycle < count_per_frame) {

                    // Вызов прерывания
                    if ((flag.i == 1) && (timer - last_timer) > intr_timer) {
                        interruptcall();
                    }

                    // Выполнение инструкции
                    framecycle += step();

                    // Проверка останова
                    for (k = 0; k < ds_brk_cnt; k++) if (pc == ds_brk[k]) { cpu_halt = 1; break; }

                    // Отладочная инструкция BREAK
                    if (cpu_halt) break;
                }

                // Остановка процессора
                if (cpu_halt) {

                    ds_debugger = 1;
                    ds_cursor = pc;
                    update_screen();
                }

                // Вращение остаточных
                framecycle %= count_per_frame;
            }

            require_halt = 0;
            flash_cnt++;

            // Имитация мерцания курсора
            if (flash_cnt == 25) {
                flash_cnt = 0;
                flash = 1 - flash;

                // Только для текстового режима
                if (videom == 0) update_screen();
            }

            flip();
        }

        SDL_Delay(1);
    }
}
