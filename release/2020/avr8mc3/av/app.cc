#include "avr.h"

// Обработчик кадра
uint AVRDisplayTimer(uint interval, void *param) {

    SDL_Event     event;
    SDL_UserEvent userevent;

    /* Создать новый Event */
    userevent.type  = SDL_USEREVENT;
    userevent.code  = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return (interval);
}

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

    count_per_frame = 200000;    // 10,0 mHz процессор
}

void APP::window(const char* caption) {

    int w = config_width;
    int h = config_height;

    width  = w;
    height = h;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_EnableUNICODE(1);

    sdl_screen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption(caption, 0);
    SDL_AddTimer(1000 / clock_video, AVRDisplayTimer, NULL);
}

APP::~APP() {
}

// Получение ASCII
int APP::get_key_ascii(SDL_Event event) {

    /* Получение ссылки на структуру с данными о нажатой клавише */
    SDL_KeyboardEvent * eventkey = & event.key;

    int xt = 0;
    int k = eventkey->keysym.scancode;

    switch (k) {

        /* A */ case 0x26: xt = 0x41; break;
        /* B */ case 0x38: xt = 0x42; break;
        /* C */ case 0x36: xt = 0x43; break;
        /* D */ case 0x28: xt = 0x44; break;
        /* E */ case 0x1a: xt = 0x45; break;
        /* F */ case 0x29: xt = 0x46; break;
        /* G */ case 0x2a: xt = 0x47; break;
        /* H */ case 0x2b: xt = 0x48; break;
        /* I */ case 0x1f: xt = 0x49; break;
        /* J */ case 0x2c: xt = 0x4A; break;
        /* K */ case 0x2d: xt = 0x4B; break;
        /* L */ case 0x2e: xt = 0x4C; break;
        /* M */ case 0x3a: xt = 0x4D; break;
        /* N */ case 0x39: xt = 0x4E; break;
        /* O */ case 0x20: xt = 0x4F; break;
        /* P */ case 0x21: xt = 0x50; break;
        /* Q */ case 0x18: xt = 0x51; break;
        /* R */ case 0x1b: xt = 0x52; break;
        /* S */ case 0x27: xt = 0x53; break;
        /* T */ case 0x1c: xt = 0x54; break;
        /* U */ case 0x1e: xt = 0x55; break;
        /* V */ case 0x37: xt = 0x56; break;
        /* W */ case 0x19: xt = 0x57; break;
        /* X */ case 0x35: xt = 0x58; break;
        /* Y */ case 0x1d: xt = 0x59; break;
        /* Z */ case 0x34: xt = 0x5A; break;

        /* 0 */ case 0x13: xt = 0x30; break;
        /* 1 */ case 0x0A: xt = 0x31; break;
        /* 2 */ case 0x0B: xt = 0x32; break;
        /* 3 */ case 0x0C: xt = 0x33; break;
        /* 4 */ case 0x0D: xt = 0x34; break;
        /* 5 */ case 0x0E: xt = 0x35; break;
        /* 6 */ case 0x0F: xt = 0x36; break;
        /* 7 */ case 0x10: xt = 0x37; break;
        /* 8 */ case 0x11: xt = 0x38; break;
        /* 9 */ case 0x12: xt = 0x39; break;

        /* ` */ case 0x31: xt = 0x60; break;
        /* - */ case 0x14: xt = 0x2D; break;
        /* = */ case 0x15: xt = 0x3D; break;
        /* \ */ case 0x33: xt = 0x5C; break;
        /* [ */ case 0x22: xt = 0x5B; break;
        /* ] */ case 0x23: xt = 0x5D; break;
        /* ; */ case 0x2f: xt = 0x3B; break;
        /* ' */ case 0x30: xt = 0x27; break;
        /* , */ case 0x3b: xt = 0x2C; break;
        /* . */ case 0x3c: xt = 0x2E; break;
        /* / */ case 0x3d: xt = 0x2F; break;

        /* F1  */ case 67: xt = key_F1; break;
        /* F2  */ case 68: xt = key_F2; break;
        /* F3  */ case 69: xt = key_F3; break;
        /* F4  */ case 70: xt = key_F4; break;
        /* F5  */ case 71: xt = key_F5; break;
        /* F6  */ case 72: xt = key_F6; break;
        /* F7  */ case 73: xt = key_F7; break;
        /* F8  */ case 74: xt = key_F8; break;
        /* F9  */ case 75: xt = key_F9; break;
        /* F10 */ case 76: xt = key_F10; break;
        /* F11 */ case 95: xt = key_F11; break;
        /* F12 */ case 96: xt = key_F12; break;

        /* bs */ case 0x16: xt = key_BS; break;     // Back Space
        /* sp */ case 0x41: xt = 0x20; break;       // Space
        /* tb */ case 0x17: xt = key_TAB; break;    // Tab
        /* ls */ case 0x32: xt = key_LSHIFT; break; // Left Shift
        /* lc */ case 0x25: xt = key_LALT;  break;  // Left Ctrl
        /* la */ case 0x40: xt = key_LCTRL; break;  // Left Alt
        /* en */ case 0x24: xt = key_ENTER; break;  // Enter
        /* es */ case 0x09: xt = key_ESC; break;    // Escape
        /* es */ case 0x08: xt = key_ESC; break;

        // ---------------------------------------------
        // Специальные (не так же как в реальном железе)
        // ---------------------------------------------

        /* UP  */  case 0x6F: xt = key_UP; break;
        /* RT  */  case 0x72: xt = key_RT; break;
        /* DN  */  case 0x74: xt = key_DN; break;
        /* LF  */  case 0x71: xt = key_LF; break;
        /* Home */ case 0x6E: xt = key_HOME; break;
        /* End  */ case 0x73: xt = key_END; break;
        /* PgUp */ case 0x70: xt = key_PGUP; break;
        /* PgDn */ case 0x75: xt = key_PGDN; break;
        /* Del  */ case 0x77: xt = key_DEL; break;
        /* Ins  */ case 0x76: xt = key_INS; break;
        /* NLock*/ case 0x4D: xt = key_NL; break;

        default: return -k;
    }

    /* Получить скан-код клавиш */
    return xt;
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
        fscanf(fp, "%d %d", & config_width, & config_height);

        // Инструкции за кадр
        count_per_frame = (clock_mhz * 1000000) / clock_video;

        fclose(fp);
    }
}

// ---------------------------------------------------------------------
// Главный обработчик приложения
// ---------------------------------------------------------------------

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

            flip();
        }

        SDL_Delay(1);
    }
}

// Положение курсора мыши X
int APP::get_mouse_x() {

    return mouse_x >> 2;// 320 x 200
}

// Положение курсора мыши Y
int APP::get_mouse_y() {

    // 320 x 200
    return mouse_y >> 2;
}

// Установить или удалить brkpoint
void APP::swi_brk() {

    int dsdel = 0; // Маркер удаления из
    for (int k = 0; k < ds_brk_cnt; k++) {

        // Удалить, if has
        if (ds_brk[k] == ds_cursor) {

            ds_brk_cnt--;
            for (int i = k; i < ds_brk_cnt; i++) {
                ds_brk[i] = ds_brk[i+1];
            }
            dsdel = 1;
            break;
        }
    }

    // Добавить точку останова, если нет ее
    if (dsdel == 0) ds_brk[ds_brk_cnt++] = ds_cursor;
}
