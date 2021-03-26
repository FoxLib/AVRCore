#include "avr.h"

// ---------------------------------------------------------------------
// ГЛАВНОЕ ПРИЛОЖЕНИЕ
// ---------------------------------------------------------------------

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
    membank         = 0;
    videomode       = 0;
    count_per_frame = 200000;    // 10,0 mHz процессор

    // Отладчик
    ds_start        = 0;
    ds_cursor       = 0;
    ds_debugger     = 1;
    ds_tab          = 0;
    ds_dump_cursor  = 0;
    ds_dump_start   = 0;
    ds_brk_cnt      = 0;

    // Процессор
    pc              = 0x0000;
    cpu_halt        = 1;
    require_halt    = 0;
    instr_counter   = 0;
    framecycle      = 0;

    // Клавиатура
    port_keyb_hit   = 0;
    key_shift       = 0;

    // Дисплей
    require_disp_update = 0;
    text_px         = 0;
    text_py         = 0;
    cursor_x        = 0;
    cursor_y        = 0;
    flash           = 0;
    flash_id        = 0;

    // SPI
    spi_status      = 0;
    spi_command     = 0;
    spi_phase       = 0;
    spi_arg         = 0;
    spi_lba         = 0;
    spi_resp        = 0xFF;
    spi_st          = 2; // Timeout SD

    // Таймер
    intr_timer      = 0;
    last_timer      = 0;

    sdram_data = (unsigned char*)malloc(64*1024*1024);
    for (int i = 0; i < 64*1024*1024; i++) sdram_data[i] = 0x55 ^ i;

    config();
    assign();
    window("AVR emulator");
}

// Деструктор
APP::~APP() { free(sdram_data); }

// ---------------------------------------------------------------------
// Главный обработчик приложения
// ---------------------------------------------------------------------

void APP::main() {

    int k, i, keyid, mk;
    int jump_dump = (height - 16*18);
    int jump_dis  = (height - 32) / 8;

    while (1) {

        int do_frame_event = 0;
        require_disp_update = 0;

        while (SDL_PollEvent(& event)) {

            switch (event.type) {

                // Закрытие приложения
                case SDL_QUIT: {
                    return;
                }

                // Регистрация движения мыши
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

                    // Установка курсора мыши
                    if (mouse_x >= 8 && mouse_x < 54*8 && mouse_y >= 16 && mouse_y < 44*16 && ds_debugger) {

                        ds_cursor = ds_addresses[ mouse_y>>4 ];
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

                    keyid = get_key(event);

                    // Режим обычной работы процессора
                    if (!ds_debugger && !cpu_halt) {

                        if (keyid == key_LSHIFT) key_shift = 1;

                        if (keyid > 0) {

                            port_keyb_hit ^= 1;
                            port_keyb_xt   = upper(keyid);
                        }
                    }

                    // F7 Процессор должен быть остановлен для некоторых команд
                    // F3 Вызов прерывания
                    if (cpu_halt) {

                        // Выполнить шаг
                        if (keyid == key_F7) { step(); ds_cursor = pc; }

                        // Вызов прерывания
                        if (keyid == key_F3) { interruptcall(); ds_cursor = pc; }

                        // Если запущена отладка, обновить
                        if (ds_debugger) ds_update();
                    }

                    // F2 Остановка
                    if (keyid == key_F2 && ds_debugger) {

                        swi_brk();
                        ds_update();
                    }

                    // F5 Посмотреть окно (рабочее) или отладчик
                    if (keyid == key_F5) {

                        ds_debugger ^= 1;
                        update_screen();
                    }

                    // F9 Запуск программы (либо остановка)
                    if (keyid == key_F9) {

                        // Процессор остановлен, запустить его
                        if (cpu_halt) {

                            cpu_halt = 0;
                            ds_debugger = 0;
                            require_disp_update = 1;
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

                        if (keyid == 'r') /* R */ ds_dump_cursor = ds_dump_start = 0x0000;
                        if (keyid == 'v') /* V */ ds_dump_cursor = ds_dump_start = 0xc000;

                        ds_update();
                    }

                    // Нажатие горящей путевки в режиме ds_tab
                    if (ds_debugger && ds_tab == 0) {

                        if (keyid == 'r') /* R */ {

                            pc = ds_start = ds_cursor = 0;
                            instr_counter = 0;
                            ds_update();
                        }
                    }

                    break;
                }

                // Отпущена клавиша
                case SDL_KEYUP: {

                    keyid = get_key(event);

                    // Клавиша отпускается
                    if (!ds_debugger && !cpu_halt) {

                        if (keyid == key_LSHIFT) key_shift = 0;

                        if (keyid > 0) {

                            port_keyb_hit ^= 1;
                            port_keyb_xt   = 0x80 | upper(keyid);
                        }
                    }

                    break;
                }

                // @TODO Вызывается по таймеру
                case SDL_USEREVENT: {

                    do_frame_event = 1;
                    break;
                }
            }
        }

        // Обрабатывается потом
        if (do_frame_event) {

            timer = (timer + 20);

            // Перерисовка полного экрана
            if (++flash_id > 10) { flash_id = 0; flash ^= 1; require_disp_update = 1; }

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

                    // Вызов прерывание
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

            // Запрос обновления экрана в режиме экрана (flash)
            if (require_disp_update && !ds_debugger && videomode == 0) { display_update(); }

            flip();
        }

        SDL_Delay(1);
    }
}

// В зависимости от нажатого SHIFT
int APP::upper(int k) {

    if (key_shift == 0) return k;
    if (k >= 'a' && k <= 'z') return k + ('A' - 'a');

    switch (k) {

        case '0':  return ')';
        case '1':  return '!';
        case '2':  return '@';
        case '3':  return '#';
        case '4':  return '$';
        case '5':  return '%';
        case '6':  return '^';
        case '7':  return '&';
        case '8':  return '*';
        case '9':  return '(';
        case '`':  return '~';
        case '-':  return '_';
        case '=':  return '+';
        case '\\': return '|';
        case '[':  return '{';
        case ']':  return '}';
        case ';':  return ':';
        case '\'': return '"';
        case ',':  return '<';
        case '.':  return '>';
        case '/':  return '?';
    }

    return k;
}

// Создание окна
void APP::window(const char* caption) {

    int w = config_width;
    int h = config_height;

    width  = w;
    height = h;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_EnableUNICODE(1);

    sdl_screen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption(caption, 0);
    SDL_EnableKeyRepeat(250, 50);

    // @TODO переделать
    SDL_AddTimer(1000 / clock_video, AVRDisplayTimer, NULL);
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

    } else {

        printf("Указанный файл не был найден\n");
        exit(1);
    }
}

// Загрузка конфигурации
void APP::config() {

    clock_mhz = 10;
    clock_video = 50;
    config_width = 1280;
    config_height = 800;

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

// Положение курсора мыши X
int APP::get_mouse_x() {

    switch (videomode) {

        case 0: return mouse_x >> 4; break; // 80 x 25
        case 3: return mouse_x >> 2; break; // 320 x 200
    }

    return 0;
}

// Положение курсора мыши Y
int APP::get_mouse_y() {

    switch (videomode) {

        case 0: return mouse_y >> 5; break; // 80 x 25
        case 3: return mouse_y >> 2; break; // 320 x 200
    }

    return 0;
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

// Получение XT-кода
int APP::get_key(SDL_Event event) {

    /* Получение ссылки на структуру с данными о нажатой клавише */
    SDL_KeyboardEvent * eventkey = & event.key;

    int xt = 0;
    int k = eventkey->keysym.scancode;

    switch (k) {

        /* A */ case 0x26: xt = 'a'; break;
        /* B */ case 0x38: xt = 'b'; break;
        /* C */ case 0x36: xt = 'c'; break;
        /* D */ case 0x28: xt = 'd'; break;
        /* E */ case 0x1a: xt = 'e'; break;
        /* F */ case 0x29: xt = 'f'; break;
        /* G */ case 0x2a: xt = 'g'; break;
        /* H */ case 0x2b: xt = 'h'; break;
        /* I */ case 0x1f: xt = 'i'; break;
        /* J */ case 0x2c: xt = 'j'; break;
        /* K */ case 0x2d: xt = 'k'; break;
        /* L */ case 0x2e: xt = 'l'; break;
        /* M */ case 0x3a: xt = 'm'; break;
        /* N */ case 0x39: xt = 'n'; break;
        /* O */ case 0x20: xt = 'o'; break;
        /* P */ case 0x21: xt = 'p'; break;
        /* Q */ case 0x18: xt = 'q'; break;
        /* R */ case 0x1b: xt = 'r'; break;
        /* S */ case 0x27: xt = 's'; break;
        /* T */ case 0x1c: xt = 't'; break;
        /* U */ case 0x1e: xt = 'u'; break;
        /* V */ case 0x37: xt = 'v'; break;
        /* W */ case 0x19: xt = 'w'; break;
        /* X */ case 0x35: xt = 'x'; break;
        /* Y */ case 0x1d: xt = 'y'; break;
        /* Z */ case 0x34: xt = 'z'; break;

        /* 0 */ case 0x13: xt = '0'; break;
        /* 1 */ case 0x0A: xt = '1'; break;
        /* 2 */ case 0x0B: xt = '2'; break;
        /* 3 */ case 0x0C: xt = '3'; break;
        /* 4 */ case 0x0D: xt = '4'; break;
        /* 5 */ case 0x0E: xt = '5'; break;
        /* 6 */ case 0x0F: xt = '6'; break;
        /* 7 */ case 0x10: xt = '7'; break;
        /* 8 */ case 0x11: xt = '8'; break;
        /* 9 */ case 0x12: xt = '9'; break;

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

// ---------------------------------------------------------------------
// ЦЕНТРАЛЬНЫЙ ПРОЦЕССОР
// ---------------------------------------------------------------------

// Байт во флаги
void APP::byte_to_flag(unsigned char f) {

    flag.c = (f >> 0) & 1;
    flag.z = (f >> 1) & 1;
    flag.n = (f >> 2) & 1;
    flag.v = (f >> 3) & 1;
    flag.s = (f >> 4) & 1;
    flag.h = (f >> 5) & 1;
    flag.t = (f >> 6) & 1;
    flag.i = (f >> 7) & 1;
};

// Флаги в байты
unsigned char APP::flag_to_byte() {

    unsigned char f =
        (flag.i<<7) |
        (flag.t<<6) |
        (flag.h<<5) |
        (flag.s<<4) |
        (flag.v<<3) |
        (flag.n<<2) |
        (flag.z<<1) |
        (flag.c<<0);

    sram[0x5F] = f;
    return f;
}

// Развернуть итоговые биты
unsigned int APP::neg(unsigned int n) {
    return n ^ 0xffff;
}

// Установка флагов
void APP::set_logic_flags(unsigned char r) {

    flag.v = 0;
    flag.n = (r & 0x80) ? 1 : 0;
    flag.s = flag.n;
    flag.z = (r == 0) ? 1 : 0;
    flag_to_byte();
}

// Флаги после вычитания с переносом
void APP::set_subcarry_flag(int d, int r, int R, int carry) {

    flag.c = d < r + carry ? 1 : 0;
    flag.z = ((R & 0xFF) == 0 && flag.z) ? 1 : 0;
    flag.n = (R & 0x80) > 1 ? 1 : 0;
    flag.v = (((d & neg(r) & neg(R)) | (neg(d) & r & R)) & 0x80) > 0 ? 1 : 0;
    flag.s = flag.n ^ flag.v;
    flag.h = (((neg(d) & r) | (r & R) | (R & neg(d))) & 0x80) > 0 ? 1 : 0;
    flag_to_byte();
}

// Флаги после вычитания
void APP::set_subtract_flag(int d, int r, int R) {

    flag.c = d < r ? 1 : 0;
    flag.z = (R & 0xFF) == 0 ? 1 : 0;
    flag.n = (R & 0x80) > 1 ? 1 : 0;
    flag.v = (((d & neg(r) & neg(R)) | (neg(d) & r & R)) & 0x80) > 0 ? 1 : 0;
    flag.s = flag.n ^ flag.v;
    flag.h = (((neg(d) & r) | (r & R) | (R & neg(d))) & 0x40) > 0 ? 1 : 0;
    flag_to_byte();
}

// Флаги после сложение с переносом
void APP::set_add_flag(int d, int r, int R, int carry) {

    flag.c = d + r + carry >= 0x100;
    flag.h = (((d & r) | (r & neg(R)) | (neg(R) & d)) & 0x08) > 0 ? 1 : 0;
    flag.z = R == 0 ? 1 : 0;
    flag.n = (R & 0x80) > 0 ? 1 : 0;
    flag.v = (((d & r & neg(R)) | (neg(d) & neg(r) & R)) & 0x80) > 0 ? 1 : 0;
    flag.s = flag.n ^ flag.v;
    flag_to_byte();
}

// Флаги после логической операции сдвига
void APP::set_lsr_flag(int d, int r) {

    flag.c = d & 1;
    flag.n = (r & 0x80) > 0 ? 1 : 0;
    flag.z = (r == 0x00) ? 1 : 0;
    flag.v = flag.n ^ flag.c;
    flag.s = flag.n ^ flag.v;
    flag_to_byte();
}

// Флаги после сложения 16 бит
void APP::set_adiw_flag(int a, int r) {

    flag.v = ((neg(a) & r) & 0x8000) > 0 ? 1 : 0;
    flag.c = ((neg(r) & a) & 0x8000) > 0 ? 1 : 0;
    flag.n = (r & 0x8000) > 0 ? 1 : 0;
    flag.z = (r & 0xFFFF) == 0 ? 1 : 0;
    flag.s = flag.v ^ flag.n;
    flag_to_byte();
}

// Флаги после вычитания 16 бит
void APP::set_sbiw_flag(int a, int r) {

    flag.v = ((neg(a) & r) & 0x8000) > 0 ? 1 : 0;
    flag.c = ((neg(a) & r) & 0x8000) > 0 ? 1 : 0;
    flag.n = (r & 0x8000) > 0 ? 1 : 0;
    flag.z = (r & 0xFFFF) == 0 ? 1 : 0;
    flag.s = flag.v ^ flag.n;
    flag_to_byte();
}

// В зависимости от инструкции CALL/JMP/LDS/STS
int APP::skip_instr() {

    switch (map[ fetch() ]) {

        case CALL:
        case JMP:
        case LDS:
        case STS:

            pc += 2;
            break;
    }

    return 2;
}

// Вызов прерывания
void APP::interruptcall() {

    flag.i = 0;
    flag_to_byte();
    last_timer = timer;
    push16(pc >> 1);
    pc = 4;
}

// Исполнение шага процессора
int APP::step() {

    int R, r, d, a, b, A, v, Z;
    unsigned short p;

    cycles  = 1;
    opcode  = fetch();
    command = map[opcode];

    // Исполнение опкода
    switch (command) {

        case WDR:
        case NOP: break;

        // Остановка выполнения кода
        case SLEEP: /* pc = pc - 2; break; */
        case BREAK: cpu_halt = 1; break;

        // Управляющие
        case RJMP:  pc = get_rjmp(); cycles = 2; break;
        case RCALL: push16(pc >> 1); pc = get_rjmp(); cycles = 3; break;
        case RET:   pc = pop16() << 1; break;
        case RETI:  pc = pop16() << 1; flag.i = 1; flag_to_byte(); break;
        case BCLR:  sram[0x5F] &= ~(1 << get_s3()); byte_to_flag(sram[0x5F]); break;
        case BSET:  sram[0x5F] |=  (1 << get_s3()); byte_to_flag(sram[0x5F]); break;

        // Условные перехдоды
        case BRBS: if ( (sram[0x5F] & (1<<(opcode & 7)))) { pc = get_branch(); cycles = 2; } break;
        case BRBC: if (!(sram[0x5F] & (1<<(opcode & 7)))) { pc = get_branch(); cycles = 2; }  break;

        // --------------------------------
        case CPSE: if (get_rd() == get_rr())              cycles = skip_instr(); break;
        case SBRC: if (!(get_rd() & (1 << (opcode & 7)))) cycles = skip_instr(); break;
        case SBRS: if   (get_rd() & (1 << (opcode & 7)))  cycles = skip_instr(); break;
        case SBIS:
        case SBIC: // Пропуск, если в порту Ap есть бит (или нет бита)

            b = (opcode & 7);
            A = (opcode & 0xF8) >> 3;
            v = get(0x20 + A) & (1 << b);
            if ((command == SBIS && v) || (command == SBIC && !v)) cycles = skip_instr();
            break;
        // --------------------------------

        // Ввод-вывод
        case IN:  put_rd(get(0x20 + get_ap())); break;
        case OUT: put(0x20 + get_ap(), get_rd()); break;

        // Сброс/установка бита в I/O
        case CBI:
        case SBI:

            b = 1 << (opcode & 0x07);
            A = (opcode & 0xF8) >> 3;

            if (command == CBI)
                 put(0x20 + A, get(0x20 + A) & ~b);
            else put(0x20 + A, get(0x20 + A) |  b);

            cycles = 2;
            break;

        // Стек
        case PUSH: push8(get_rd()); cycles = 2; break;
        case POP:  put_rd(pop8()); cycles = 2; break;

        // Срециальные
        case SWAP: d = get_rd(); put_rd(((d & 0x0F) << 4) + ((d & 0xF0) >> 4)); break;
        case BST:  flag.t = (get_rd() & (1 << (opcode & 7))) > 0 ? 1 : 0; flag_to_byte(); break;
        case BLD:  a = get_rd(); b = (1 << (opcode & 7)); put_rd( flag.t ? (a | b) : (a & ~b) ); break;

        // =============================================================
        // Арифметико-логические инструкции
        // =============================================================

        // Вычитание с переносом, но без записи
        case CPC:

            d = get_rd();
            r = get_rr();
            R = (d - r - flag.c) & 0xff;
            set_subcarry_flag(d, r, R, flag.c);
            break;

        // Вычитание с переносом
        case SBC:

            d = get_rd();
            r = get_rr();
            R = (d - r - flag.c) & 0xFF;
            set_subcarry_flag(d, r, R, flag.c);
            put_rd(R);
            break;

        // Сложение без переноса
        case ADD:

            d = get_rd();
            r = get_rr();
            R = (d + r) & 0xff;
            set_add_flag(d, r, R, 0);
            put_rd(R);
            break;

        // Вычитание без переноса
        case CP:

            d = get_rd();
            r = get_rr();
            R = (d - r) & 0xFF;
            set_subtract_flag(d, r, R);
            break;

        // Вычитание без переноса
        case SUB:

            d = get_rd();
            r = get_rr();
            R = (d - r) & 0xFF;
            set_subtract_flag(d, r, R);
            put_rd(R);
            break;

        // Сложение с переносом
        case ADC:

            d = get_rd();
            r = get_rr();
            R = (d + r + flag.c) & 0xff;
            set_add_flag(d, r, R, flag.c);
            put_rd(R);
            break;

        case AND: R = get_rd() & get_rr(); set_logic_flags(R); put_rd(R); break;
        case OR:  R = get_rd() | get_rr(); set_logic_flags(R); put_rd(R); break;
        case EOR: R = get_rd() ^ get_rr(); set_logic_flags(R); put_rd(R); break;

        // Логическое умножение
        case ANDI: R = get_rdi() & get_imm8(); set_logic_flags(R); put_rdi(R); break;
        case ORI:  R = get_rdi() | get_imm8(); set_logic_flags(R); put_rdi(R); break;

        // Вычитание непосредственного значения
        case SUBI:

            d = get_rdi();
            r = get_imm8();
            R = (d - r) & 0xFF;
            set_subtract_flag(d, r, R);
            put_rdi(R);
            break;

        // Вычитание непосредственного значения с переносом
        case SBCI:

            d = get_rdi();
            r = get_imm8();
            R = (d - r - flag.c) & 0xFF;
            set_subcarry_flag(d, r, R, flag.c);
            put_rdi(R);
            break;

        // Сравнение без переноса
        case CPI:

            d = get_rdi();
            r = get_imm8();
            R = (d - r) & 0xFF;
            set_subtract_flag(d, r, R);
            break;

        // Развернуть биты в другую сторону
        case COM:

            d = get_rd();
            r = (d ^ 0xFF) & 0xFF;
            set_logic_flags(r);
            flag.c = 1; flag_to_byte();
            put_rd(r);
            break;

        // Декремент
        case DEC:

            d = get_rd();
            r = (d - 1) & 0xff;
            put_rd(r);

            flag.v = (r == 0x7F) ? 1 : 0;
            flag.n = (r & 0x80) > 0 ? 1 : 0;
            flag.z = (r == 0x00) ? 1 : 0;
            flag.s = flag.v ^ flag.n;
            flag_to_byte();
            break;

        // Инкремент
        case INC:

            d = get_rd();
            r = (d + 1) & 0xff;
            put_rd(r);

            flag.v = (r == 0x80) ? 1 : 0;
            flag.n = (r & 0x80) > 0 ? 1 : 0;
            flag.z = (r == 0x00) ? 1 : 0;
            flag.s = flag.v ^ flag.n;
            flag_to_byte();
            break;

        // Сложение 16-битного числа
        case ADIW:

            d = 24 + ((opcode & 0x30) >> 3);
            a = get16(d);
            r = a + get_ka();
            set_adiw_flag(a, r);
            put16(d, r);
            cycles = 2;
            break;

        // Вычитание 16-битного числа
        case SBIW:

            d = 24 + ((opcode & 0x30) >> 3);
            a = get16(d);
            r = a - get_ka();
            set_sbiw_flag(a, r);
            put16(d, r);
            cycles = 2;
            break;

        // Логический сдвиг вправо
        case LSR:

            d = get_rd();
            r = d >> 1;
            set_lsr_flag(d, r);
            put_rd(r);
            break;

        // Арифметический вправо
        case ASR:

            d = get_rd();
            r = (d >> 1) | (d & 0x80);
            set_lsr_flag(d, r);
            put_rd(r);
            break;

        // Циклический сдвиг вправо
        case ROR:

            d = get_rd();
            r = (d >> 1) | (flag.c ? 0x80 : 0x00);
            set_lsr_flag(d, r);
            put_rd(r);
            break;

        // Отрицание
        case NEG:

            d = get_rd();
            R = (-d) & 0xFF;
            set_subtract_flag(0, d, R);
            put_rd(R);
            break;

        // =============================================================
        // Перемещения
        // =============================================================
        case LDI: put_rdi(get_imm8()); break;

        // Загрузка из памяти в регистры
        case LPM0Z:  sram[0] = program[get_Z()]; cycles = 3; break;
        case LPMRZ:  put_rd(program[get_Z()]); cycles = 3; break;
        case LPMRZ_: p = get_Z(); put_rd(program[p]); put_Z(p+1); cycles = 3; break;

        // Store X
        case STX:   put(get_X(), get_rd()); cycles = 2; break;
        case STX_:  p = get_X();     put(p, get_rd()); put_X(p+1); cycles = 2; break;
        case ST_X:  p = get_X() - 1; put(p, get_rd()); put_X(p); cycles = 2; break;

        // Store Y
        case STYQ:  put((get_Y() + get_qi()), get_rd()); cycles = 2; break;
        case STY_:  p = get_Y();     put(p, get_rd()); put_Y(p+1); cycles = 2; break;
        case ST_Y:  p = get_Y() - 1; put(p, get_rd()); put_Y(p); cycles = 2; break;

        // Store Z
        case STZQ:  put((get_Z() + get_qi()), get_rd()); cycles = 2; break;
        case STZ_:  p = get_Z();     put(p, get_rd()); put_Z(p+1); cycles = 2; break;
        case ST_Z:  p = get_Z() - 1; put(p, get_rd()); put_Z(p); cycles = 2; break;

        // Load X
        case LDX:   put_rd(get(get_X())); break;
        case LDX_:  p = get_X();     put_rd(get(p)); put_X(p+1); break;
        case LD_X:  p = get_X() - 1; put_rd(get(p)); put_X(p); break;

        // Load Y
        case LDYQ:  put_rd(get((get_Y() + get_qi()))); cycles = 2; break;
        case LDY_:  p = get_Y();     put_rd(get(p)); put_Y(p+1); cycles = 2; break;
        case LD_Y:  p = get_Y() - 1; put_rd(get(p)); put_Y(p); cycles = 2; break;

        // Load Z
        case LDZQ:  put_rd(get((get_Z() + get_qi()))); cycles = 2; break;
        case LDZ_:  p = get_Z();     put_rd(get(p)); put_Z(p+1); cycles = 2; break;
        case LD_Z:  p = get_Z() - 1; put_rd(get(p)); put_Z(p); cycles = 2; break;

        case MOV:   put_rd(get_rr()); break;
        case MOVW:

            r = (get_rr_index() & 0xF) << 1;
            d = (get_rd_index() & 0xF) << 1;

            put16(d, get16(r));
            break;

        case LDS: d = fetch(); put_rd( get(d) ); break;
        case STS: d = fetch(); put(d, get_rd()); break;

        // Загрузка из доп. памяти
        case ELPM0Z:  sram[0] = program[ get_Z() + (sram[0x5B] << 16) ]; cycles = 3; break; break;
        case ELPMRZ:  put_rd(program[ get_Z() + (sram[0x5B] << 16) ]); break;
        case ELPMRZ_: p = get_Z() + (sram[0x5B] << 16); put_rd(program[p]); put_Z(p+1); cycles = 3; break;

        // ------------ РАСШИРЕНИЯ -------------------------------------

        /*
        // Логические операции между (Z) и Rd
        case LAC:

            Z = get_Z();
            put(Z, get(Z) & (get_rd() ^ 0xFF));
            break;

        case LAS:

            Z = get_Z();
            put(Z, get(Z) | get_rd());
            break;

        case LAT:

            Z = get_Z();
            put(Z, get(Z) ^ get_rd());
            break;

        // Обмен (Z) и Rd
        case XCH:

            p = get_Z();
            r = get(p);
            put(p, get_rd());
            put_rd(r);
            break;
        */

        case JMP:

            pc = 2 * ((get_jmp() << 16) | fetch());
            cycles = 3;
            break;

        case CALL:

            push16((pc + 2) >> 1);
            pc = 2 * ((get_jmp() << 16) | fetch());
            cycles = 4;
            break;

        default:

            printf("Неизвестная инструкция $%04x в pc=$%04x\n", opcode, pc - 2);
            exit(1);
    }

    instr_counter += cycles;
    return cycles;
}

// ---------------------------------------------------------------------
// СОЗДАНИЕ МАСКИ ОПКОДОВ
// ---------------------------------------------------------------------

// Применение маски к опкоду
void APP::assign_mask(const char* mask, int opcode) {

    int i, j;
    int mk[16], nmask = 0, baseop = 0;

    // Расчет позиции, где находится маска
    for (i = 0; i < 16; i++) {

        int ch = mask[15-i];
        if (ch == '0' || ch == '1')
            baseop |= (ch == '1' ? 1 << i : 0);
        else
            mk[nmask++] = i;
    }

    // Перебрать 2^nmask вариантов, по количеству битов и назначить всем опкод
    for (i = 0; i < (1<<nmask); i++) {

        int instn = baseop;
        for (j = 0; j < nmask; j++) if (i & (1 << j)) instn |= (1 << mk[j]);
        map[instn] = opcode;
    }
}

// Создание инструкции
void APP::assign() {

    int i;

    for (i = 0; i < 65536; i++) { sram[i] = i < 32 ? 0xFF : 0; program[i] = 0; }
    for (i = 0; i < 65536; i++) { map[i] = UNDEFINED; }

    sram[0x5D] = 0xFF;
    sram[0x5E] = 0xEF;
    sram[0x5F] = 0x00; // SREG

    // Для отладчика
    for (i = 0; i < 96; i++) pvsram[i] = sram[i];

    // Очистка флагов
    flag.c = 0; flag.s = 0;
    flag.z = 0; flag.h = 0;
    flag.n = 0; flag.t = 0;
    flag.v = 0; flag.i = 0;

    // Копировать DOS палитру
    for (i = 0; i < 16; i++) {

        int bc = (DOS_13[i] & 0xF0) >> 4;
        int gc = (DOS_13[i] & 0xF000) >> 8;
        int rc = (DOS_13[i] & 0xF00000) >> 20;

        sram[BASE_TEXT + 0xFA0 + 2*i] = bc | gc;
        sram[BASE_TEXT + 0xFA1 + 2*i] = rc;
    }

    // Скопировать FontROM (Bank 1)
    for (i = 0; i < 4096; i++) {
        sram[BASE_TEXT + 0x1000 + i] = ansi16[i >> 4][i & 15];
    }

    // Арифметические на 2 регистра
    assign_mask("000001rdddddrrrr", CPC);   // +
    assign_mask("000010rdddddrrrr", SBC);   // +
    assign_mask("000011rdddddrrrr", ADD);   // +
    assign_mask("000101rdddddrrrr", CP);    // +
    assign_mask("000110rdddddrrrr", SUB);   // +
    assign_mask("000111rdddddrrrr", ADC);   // +
    assign_mask("001000rdddddrrrr", AND);   // +
    assign_mask("001001rdddddrrrr", EOR);   // +
    assign_mask("001010rdddddrrrr", OR);    // +

    // Арифметика регистр + операнд K
    assign_mask("0011KKKKddddKKKK", CPI);   // +
    assign_mask("0100KKKKddddKKKK", SBCI);  // +
    assign_mask("0101KKKKddddKKKK", SUBI);  // +
    assign_mask("0110KKKKddddKKKK", ORI);   // +
    assign_mask("0111KKKKddddKKKK", ANDI);  // +
    assign_mask("10010110KKddKKKK", ADIW);  // +
    assign_mask("10010111KKddKKKK", SBIW);  // +

    // Условные и безусловные переходы
    assign_mask("1100kkkkkkkkkkkk", RJMP);  // +
    assign_mask("1101kkkkkkkkkkkk", RCALL); // +
    assign_mask("1001010100001000", RET);   // +
    assign_mask("1001010100011000", RETI);  // +
    assign_mask("111100kkkkkkksss", BRBS);  // +
    assign_mask("111101kkkkkkksss", BRBC);  // +
    assign_mask("1111110ddddd0bbb", SBRC);  // +
    assign_mask("1111111ddddd0bbb", SBRS);  // +
    assign_mask("10011001AAAAAbbb", SBIC);  // +
    assign_mask("10011011AAAAAbbb", SBIS);  // +
    assign_mask("000100rdddddrrrr", CPSE);  // +

    // Непрямые и длинные переходы
    assign_mask("1001010100001001", ICALL);
    assign_mask("1001010100011001", EICALL);
    assign_mask("1001010000001001", IJMP);
    assign_mask("1001010000011001", EIJMP);
    assign_mask("1001010kkkkk111k", CALL);  // +
    assign_mask("1001010kkkkk110k", JMP);   // +

    // Перемещения
    assign_mask("1110KKKKddddKKKK", LDI);   // +
    assign_mask("001011rdddddrrrr", MOV);   // +
    assign_mask("1001000ddddd0000", LDS);   // +
    assign_mask("1001001ddddd0000", STS);   // +
    assign_mask("00000001ddddrrrr", MOVW);  // +
    assign_mask("1111100ddddd0bbb", BLD);   // +
    assign_mask("1111101ddddd0bbb", BST);   // +
    assign_mask("1001001ddddd0100", XCH);   // +

    // Однооперандные
    assign_mask("1001010ddddd0011", INC);   // +
    assign_mask("1001010ddddd1010", DEC);   // +
    assign_mask("1001010ddddd0110", LSR);   // +
    assign_mask("1001010ddddd0101", ASR);   // +
    assign_mask("1001010ddddd0111", ROR);   // +
    assign_mask("1001010ddddd0001", NEG);   // +
    assign_mask("1001010ddddd0000", COM);   // +
    assign_mask("1001010ddddd0010", SWAP);  // +
    assign_mask("1001001ddddd0110", LAC);   // +
    assign_mask("1001001ddddd0101", LAS);   // +
    assign_mask("1001001ddddd0111", LAT);   // +
    assign_mask("100101001sss1000", BCLR);  // +
    assign_mask("100101000sss1000", BSET);  // +
    assign_mask("10011000AAAAAbbb", CBI);   // +
    assign_mask("10011010AAAAAbbb", SBI);   // +

    // Косвенная загрузка из памяти
    assign_mask("1001000ddddd1100", LDX);   // +
    assign_mask("1001000ddddd1101", LDX_);  // +
    assign_mask("1001000ddddd1110", LD_X);  // +
    assign_mask("1001000ddddd1001", LDY_);  // +
    assign_mask("1001000ddddd1010", LD_Y);  // +
    assign_mask("10q0qq0ddddd1qqq", LDYQ);  // +
    assign_mask("1001000ddddd0001", LDZ_);  // +
    assign_mask("1001000ddddd0010", LD_Z);  // +
    assign_mask("10q0qq0ddddd0qqq", LDZQ);  // +

    // Косвенное сохранение
    assign_mask("1001001ddddd1100", STX);   // +
    assign_mask("1001001ddddd1101", STX_);  // +
    assign_mask("1001001ddddd1110", ST_X);  // +
    assign_mask("1001001ddddd1001", STY_);  // +
    assign_mask("1001001ddddd1010", ST_Y);  // +
    assign_mask("10q0qq1ddddd1qqq", STYQ);  // +
    assign_mask("1001001ddddd0001", STZ_);  // +
    assign_mask("1001001ddddd0010", ST_Z);  // +
    assign_mask("10q0qq1ddddd0qqq", STZQ);  // +

    // Загрузка из запись в память программ
    assign_mask("1001010111001000", LPM0Z);  // +
    assign_mask("1001000ddddd0100", LPMRZ);  // +
    assign_mask("1001000ddddd0101", LPMRZ_); // +
    assign_mask("1001010111101000", SPM);
    assign_mask("1001010111111000", SPM2);

    // Особые инструкции расширенной загрузки из памяти
    assign_mask("1001010111011000", ELPM0Z);  // +
    assign_mask("1001000ddddd0110", ELPMRZ);  // +
    assign_mask("1001000ddddd0111", ELPMRZ_); // +

    // Специальные
    assign_mask("1001010110001000", SLEEP); // +
    assign_mask("1001010110101000", WDR);   // +
    assign_mask("1001010110011000", BREAK); // +
    assign_mask("0000000000000000", NOP);   // +
    assign_mask("10110AAdddddAAAA", IN);    // +
    assign_mask("10111AAdddddAAAA", OUT);   // +
    assign_mask("1001001ddddd1111", PUSH);  // +
    assign_mask("1001000ddddd1111", POP);   // +
    assign_mask("10010100KKKK1011", DES);
}

// ---------------------------------------------------------------------
// ДИЗАССЕМБЛЕР
// ---------------------------------------------------------------------

// Прочесть следующий опкод
int APP::ds_fetch(uint& addr) {

    int L = program[addr];
    addr = (addr + 1) & (128*1024 - 1);

    int H = program[addr];
    addr = (addr + 1) & (128*1024 - 1);

    return H*256 + L;
}

// Дизассемблировать адрес
int APP::ds_info(uint addr) {

    int pvaddr = addr;

    // Прочесть опкод
    int opcode = ds_fetch(addr);

    // Непосредственное значение, если нужно
    int K = ((opcode & 0xF00) >> 4) | (opcode & 0xF);

    // Номера регистров и портов
    int Rd = (opcode & 0x1F0) >> 4;
    int Rr = (opcode & 0x00F) | ((opcode & 0x200) >> 5);
    int Ap = (opcode & 0x00F) | ((opcode & 0x600) >> 5);
    int Ka = (opcode & 0x00F) | ((opcode & 0x0C0) >> 2);

    // ADDR[7:0] = (~INST[8], INST[8], INST[10], INST[9], INST[3], INST[2], INST[1], INST[0])
    int Ld = (opcode & 0x00F) | ((opcode & 0x600) >> 5) | ((opcode & 0x100) >> 2) | (((~opcode) & 0x100) >> 1);

    // 00q0 qq00 0000 0qqq
    int Qi = (opcode & 0x007) | ((opcode & 0xC00)>>7) | ((opcode & 0x2000) >> 8);

    // Относительный переход
    int Rjmp = addr + 2*((opcode & 0x800) > 0 ? (opcode & 0x7FF) - 0x800 : (opcode & 0x7FF));
    int Bjmp = addr + 2*((opcode & 0x200) > 0 ? ((opcode & 0x1F8)>>3) - 0x40 : ((opcode & 0x1F8)>>3) );
    int Bit7 = opcode & 7;
    int bit7s = (opcode & 0x70) >> 4;
    int jmpfar = (((opcode & 0x1F0) >> 3) | (opcode & 1)) << 16;
    int append;

    // Получение всевозможных мнемоник
    char name_Rd[32];   sprintf(name_Rd,    "r%d", Rd);
    char name_Rr[32];   sprintf(name_Rr,    "r%d", Rr);
    char name_Rdi[32];  sprintf(name_Rdi,   "r%d", (0x10 | Rd));
    char name_K[32];    sprintf(name_K,     "$%02X", K);
    char name_Ap[32];   sprintf(name_Ap,    "$%02X", Ap);
    char name_Ap8[32];  sprintf(name_Ap8,   "$%02X", (opcode & 0xF8) >> 3);
    char name_rjmp[32]; sprintf(name_rjmp,  "$%04X", Rjmp);
    char name_bjmp[32]; sprintf(name_bjmp,  "$%04X", Bjmp);
    char name_lds[32];  sprintf(name_lds,   "$%02X", Ld);
    char name_Rd4[32];  sprintf(name_Rd4,   "r%d", (Rd & 0xF)*2);
    char name_Rr4[32];  sprintf(name_Rr4,   "r%d", (Rr & 0xF)*2);
    char name_adiw[32]; sprintf(name_adiw,  "r%d", 24 + ((opcode & 0x30)>>3));
    char name_Ka[32];   sprintf(name_Ka,    "$%02X", Ka);
    char name_bit7[4];  sprintf(name_bit7,  "%d", Bit7);

    // Смещение
    char name_Yq[32];   if (Qi) sprintf(name_Yq, "Y+%d", Qi); else sprintf(name_Yq, "Y");
    char name_Zq[32];   if (Qi) sprintf(name_Zq, "Z+%d", Qi); else sprintf(name_Zq, "Z");
    char name_LDD[32];  if (Qi) sprintf(name_LDD, "ldd"); else sprintf(name_LDD, "ld");
    char name_STD[32];  if (Qi) sprintf(name_STD, "std"); else sprintf(name_STD, "st");

    // Условие
    char name_brc[16];  sprintf(name_brc, "%s", ds_brcs[0][Bit7]);
    char name_brs[16];  sprintf(name_brs, "%s", ds_brcs[1][Bit7]);

    // Вывод
    char mnem[32], op1[32], op2[32]; op1[0] = 0; op2[0] = 0; sprintf(mnem, "<unk>");

    // Store/Load indirect LDD, STD
    switch (opcode & 0xD208) {

        case 0x8000: strcpy(mnem, name_LDD); strcpy(op1, name_Rd); strcpy(op2, name_Zq); break;
        case 0x8008: strcpy(mnem, name_LDD); strcpy(op1, name_Rd); strcpy(op2, name_Yq); break;
        case 0x8200: strcpy(mnem, name_STD); strcpy(op1, name_Zq); strcpy(op2, name_Rd); break;
        case 0x8208: strcpy(mnem, name_STD); strcpy(op1, name_Yq); strcpy(op2, name_Rd); break;
    }

    // Immediate
    switch (opcode & 0xF000) {

        case 0xC000: strcpy(mnem, "rjmp");  strcpy(op1, name_rjmp); break; // k
        case 0xD000: strcpy(mnem, "rcall"); strcpy(op1, name_rjmp); break; // k
        case 0xE000: strcpy(mnem, "ldi");   strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
        case 0x3000: strcpy(mnem, "cpi");   strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
        case 0x4000: strcpy(mnem, "sbci");  strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
        case 0x5000: strcpy(mnem, "subi");  strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
        case 0x6000: strcpy(mnem, "ori");   strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
        case 0x7000: strcpy(mnem, "andi");  strcpy(op1, name_Rdi); strcpy(op2, name_K); break; // Rd, K
    }

    // lds/sts 16 bit, in/out
    switch (opcode & 0xF800) {

        case 0xB000: strcpy(mnem, "in");  strcpy(op1, name_Rd);  strcpy(op2, name_Ap);  break; // Rd, A
        case 0xB800: strcpy(mnem, "out"); strcpy(op1, name_Ap);  strcpy(op2, name_Rd);  break; // A, Rr
    }

    // bset/clr
    switch (opcode & 0xFF8F) {

        case 0x9408: strcpy(mnem, ds_brcs[2][ bit7s ]); break;
        case 0x9488: strcpy(mnem, ds_brcs[3][ bit7s ]); break;
    }

    // alu op1, op2
    switch (opcode & 0xFC00) {

        case 0x0400: strcpy(mnem, "cpc");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x0800: strcpy(mnem, "sbc");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x0C00: strcpy(mnem, "add");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x1C00: // Rd, Rr

            strcpy(mnem, Rd == Rr ? "rol" : "adc");
            strcpy(op1, name_Rd);
            if (Rr != Rd) { strcpy(op2, name_Rr); }
            break;

        case 0x2C00: strcpy(mnem, "mov");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x9C00: strcpy(mnem, "mul");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x1000: strcpy(mnem, "cpse"); strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x1400: strcpy(mnem, "cp");   strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x1800: strcpy(mnem, "sub");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x2000: strcpy(mnem, "and");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x2400: strcpy(mnem, "eor");  strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0x2800: strcpy(mnem, "or");   strcpy(op1, name_Rd); strcpy(op2, name_Rr); break; // Rd, Rr
        case 0xF000: sprintf(mnem, "br%s", name_brs); strcpy(op1, name_bjmp); break; // s, k
        case 0xF400: sprintf(mnem, "br%s", name_brc); strcpy(op1, name_bjmp); break; // s, k
    }

    // Bit operation
    switch (opcode & 0xFE08) {

        case 0xF800: strcpy(mnem, "bld");   strcpy(op1, name_Rd); strcpy(op2, name_bit7); break; // Rd, b
        case 0xFA00: strcpy(mnem, "bst");   strcpy(op1, name_Rd); strcpy(op2, name_bit7); break; // Rd, b
        case 0xFC00: strcpy(mnem, "sbrc");  strcpy(op1, name_Rd); strcpy(op2, name_bit7); break; // Rr, b
        case 0xFE00: strcpy(mnem, "sbrs");  strcpy(op1, name_Rd); strcpy(op2, name_bit7); break; // Rr, b
    }

    // jmp/call 24 bit
    switch (opcode & 0xFE0E) {

        case 0x940C: strcpy(mnem, "jmp");  append = ds_fetch(addr); sprintf(op1, "%05X", 2*(jmpfar + append)); break; // k2
        case 0x940E: strcpy(mnem, "call"); append = ds_fetch(addr); sprintf(op1, "%05X", 2*(jmpfar + append)); break; // k2
    }

    // ST/LD
    switch (opcode & 0xFE0F) {

        case 0x900F: strcpy(mnem, "pop");   strcpy(op1, name_Rd); break; // Rd
        case 0x920F: strcpy(mnem, "push");  strcpy(op1, name_Rd); break; // Rd
        case 0x940A: strcpy(mnem, "dec");   strcpy(op1, name_Rd); break; // Rd
        case 0x9204: strcpy(mnem, "xch");   strcpy(op1, name_Rd); break; // Rr
        case 0x9205: strcpy(mnem, "las");   strcpy(op1, name_Rd); break; // Rr
        case 0x9206: strcpy(mnem, "lac");   strcpy(op1, name_Rd); break; // Rr
        case 0x9207: strcpy(mnem, "lat");   strcpy(op1, name_Rd); break; // Rr
        case 0x9400: strcpy(mnem, "com");   strcpy(op1, name_Rd); break; // Rd
        case 0x9401: strcpy(mnem, "neg");   strcpy(op1, name_Rd); break; // Rd
        case 0x9402: strcpy(mnem, "swap");  strcpy(op1, name_Rd); break; // Rd
        case 0x9403: strcpy(mnem, "inc");   strcpy(op1, name_Rd); break; // Rd
        case 0x9405: strcpy(mnem, "asr");   strcpy(op1, name_Rd); break; // Rd
        case 0x9406: strcpy(mnem, "lsr");   strcpy(op1, name_Rd); break; // Rd
        case 0x9407: strcpy(mnem, "ror");   strcpy(op1, name_Rd); break; // Rd
        case 0x900A: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "-Y"); break; // Rd, -Y
        case 0x900C: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "X");  break; // Rd, X
        case 0x900D: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "X+"); break; // Rd, X+
        case 0x900E: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "-X"); break; // Rd, -X
        case 0x9001: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "Z+"); break; // Rd, Z+
        case 0x9002: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "-Z"); break; // Rd, -Z
        case 0x9004: strcpy(mnem, "lpm");   strcpy(op1, name_Rd); strcpy(op2, "Z");  break; // Rd, Z
        case 0x9005: strcpy(mnem, "lpm");   strcpy(op1, name_Rd); strcpy(op2, "Z+"); break; // Rd, Z+
        case 0x9006: strcpy(mnem, "elpm");  strcpy(op1, name_Rd); strcpy(op2, "Z");  break; // Rd, Z
        case 0x9007: strcpy(mnem, "elpm");  strcpy(op1, name_Rd); strcpy(op2, "Z+"); break; // Rd, Z+
        case 0x9009: strcpy(mnem, "ld");    strcpy(op1, name_Rd); strcpy(op2, "Y+"); break; // Rd, Y+
        case 0x920C: strcpy(mnem, "st");    strcpy(op1, "X");  strcpy(op2, name_Rd); break; // X, Rd
        case 0x920D: strcpy(mnem, "st");    strcpy(op1, "X+"); strcpy(op2, name_Rd); break; // X+, Rd
        case 0x920E: strcpy(mnem, "st");    strcpy(op1, "-X"); strcpy(op2, name_Rd); break; // -X, Rd
        case 0x9209: strcpy(mnem, "std");   strcpy(op1, "Y+"); strcpy(op2, name_Rd); break; // Y+, Rd
        case 0x920A: strcpy(mnem, "std");   strcpy(op1, "-Y"); strcpy(op2, name_Rd); break; // -Y, Rd
        case 0x9201: strcpy(mnem, "std");   strcpy(op1, "Z+"); strcpy(op2, name_Rd); break; // Z+, Rd
        case 0x9202: strcpy(mnem, "std");   strcpy(op1, "-Z"); strcpy(op2, name_Rd); break; // -Z, Rd

        case 0x9000:
        case 0x9200:

            strcpy(mnem, opcode & 0x0200 ? "sts" : "lds");
            strcpy(op1, name_Rd);
            sprintf(op2, "$%04X", ds_fetch(addr));
            break;
    }

    // Word Ops
    switch (opcode & 0xFF00) {

        case 0x0100: strcpy(mnem, "movw"); strcpy(op1, name_Rd4);  strcpy(op2, name_Rr4);  break; // Rd+1:Rd, Rr+1:Rr
        case 0x0200: strcpy(mnem, "muls"); strcpy(op1, name_Rd);   strcpy(op2, name_Rr);   break; // Rd, Rr
        case 0x9A00: strcpy(mnem, "sbi");  strcpy(op1, name_Ap8);  strcpy(op2, name_bit7); break; // A, b
        case 0x9B00: strcpy(mnem, "sbis"); strcpy(op1, name_Ap8);  strcpy(op2, name_bit7); break; // A, b
        case 0x9600: strcpy(mnem, "adiw"); strcpy(op1, name_adiw); strcpy(op2, name_Ka);   break; // Rd+1:Rd, K
        case 0x9700: strcpy(mnem, "sbiw"); strcpy(op1, name_adiw); strcpy(op2, name_Ka);   break; // Rd+1:Rd, K
        case 0x9800: strcpy(mnem, "cbi");  strcpy(op1, name_Ap8);  strcpy(op2, name_bit7); break; // A, b
        case 0x9900: strcpy(mnem, "sbic"); strcpy(op1, name_Ap8);  strcpy(op2, name_bit7); break; // A, b
    }

    // DES
    switch (opcode & 0xFF0F) {

        case 0x940B: strcpy(mnem, "des");  sprintf(op1, "$%02X", (opcode & 0xF0) >> 4); break; // K
    }

    // Multiply
    switch (opcode & 0xFF88) {

        case 0x0300: strcpy(mnem, "mulsu");  strcpy(op1, name_Rd); strcpy(op1, name_Rr); break; // Rd, Rr
        case 0x0308: strcpy(mnem, "fmul");   strcpy(op1, name_Rd); strcpy(op1, name_Rr); break; // Rd, Rr
        case 0x0380: strcpy(mnem, "fmuls");  strcpy(op1, name_Rd); strcpy(op1, name_Rr); break; // Rd, Rr
        case 0x0388: strcpy(mnem, "fmulsu"); strcpy(op1, name_Rd); strcpy(op1, name_Rr); break; // Rd, Rr
    }

    // Одиночные
    switch (opcode & 0xFFFF) {

        case 0x0000: strcpy(mnem, "nop");   break;
        case 0x95A8: strcpy(mnem, "wdr");   break;
        case 0x95C8: strcpy(mnem, "lpm");   strcpy(op1, "r0"); strcpy(op2, "Z"); break; // R0, Z
        case 0x95D8: strcpy(mnem, "elpm");  strcpy(op1, "r0"); strcpy(op2, "Z"); break; // R0, Z
        case 0x9409: strcpy(mnem, "ijmp");  break;
        case 0x9419: strcpy(mnem, "eijmp"); break;
        case 0x9508: strcpy(mnem, "ret");   break;
        case 0x9509: strcpy(mnem, "icall"); break;
        case 0x9518: strcpy(mnem, "reti");  break;
        case 0x9519: strcpy(mnem, "eicall"); break;
        case 0x95E8: strcpy(mnem, "spm");   break;
        case 0x95F8: strcpy(mnem, "spm.2"); break;
        case 0x9588: strcpy(mnem, "sleep"); break;
        case 0x9598: strcpy(mnem, "break"); break;
    }

    // Дополнить пробелами
    int l_mnem = strlen(mnem);
    int l_op1  = strlen(op1);
    int l_op2  = strlen(op2);

    for (int i = l_mnem; i < 8; i++) strcat(mnem, " ");

    // Вывести строку
    if (l_op1 && l_op2) {
        sprintf(ds_line, "%s %s, %s", mnem, op1, op2);
    } else if (l_op1) {
        sprintf(ds_line, "%s %s", mnem, op1);
    } else {
        sprintf(ds_line, "%s", mnem);
    }

    return addr - pvaddr;
}

// Обновить экран с дизассемблированием
void APP::ds_update() {

    int  size, cl, k;
    char tmp[32];
    int  catched = 0;

    int  wx = width  / 8;
    int  wy = height / 16;

    cls(0x000040);
    ds_current = ds_start;

    // Отрисовка дизассемблера
    for (int i = 1; i < wy-1; i++) {

        int hlight = 0;

        // Декодирование
        size = ds_info(ds_current);

        // Запись на всякий пожарный
        ds_addresses[i] = ds_current;

        // Нарисовать линию
        if (ds_cursor == ds_current) {

            // Показать только если табуляция тут есть
            if (ds_tab == 0) {

                for (int a = 0; a < 16; a++)
                for (int b = 8; b < 8*(wx-42); b++)
                    pset(b, i*16 + a, 0x0080ff);

                hlight = 1;
            }
            catched = 1;
        }

        // Адрес
        sprintf(tmp, "%05X", ds_current); print(2, i, tmp, hlight ? 0xffff00 : 0xe0e000);

        // Обычный размер (size = 4)
        sprintf(tmp, "%04X", program[ds_current] + 256*program[ds_current+1]); print(8, i, tmp, hlight ? 0xf0f0f0 : 0xa0a0a0);

        // Расширенный размер
        if (size == 4) {

            sprintf(tmp, "%04X", program[ds_current+2] + 256*program[ds_current+3]);
            print(13, i, tmp, hlight ? 0xf0f0f0 : 0xa0a0a0);
        }

        // Мнемоника и аргумент
        print(18, i, ds_line, hlight ? 0xffffff : 0xe0e0e0);

        // Точка текущего положения
        if (ds_current == pc) print(1, i, "\x10", 0xfff80);

        // Тест на брейкпоинты
        for (k = 0; k < ds_brk_cnt; k++) if (ds_current == ds_brk[k]) print(7, i, "\x0A", 0xff00000);

        // К следующей
        ds_current += size;
    }

    // Если курсор не отловлен, перерисовать заново с новой позиции
    if (catched == 0) { ds_start = ds_cursor; ds_update(); }

    // Рамки
    print16char(0,      0,      0xC9, 0xc0c0c0);
    print16char(0,      wy-1,   0xC8, 0xc0c0c0);
    print16char(wx-42,  0,      0xCB, 0xc0c0c0);
    print16char(wx-42,  wy-1,   0xCA, 0xc0c0c0);
    print16char(wx-1,   0,      0xBB, 0xc0c0c0);
    print16char(wx-1,   wy-1,   0xBC, 0xc0c0c0);

    for (int i = 1; i < wy-1;  i++) { print16char(0, i, 0xBA, 0xc0c0c0); print16char(wx-42, i, 0xBA, 0xc0c0c0); print16char(wx-1, i, 0xBA, 0xc0c0c0); }
    for (int i = 1; i < wx-42; i++) { print16char(i, 0, 0xCD, 0xc0c0c0); print16char(i, wy-1, 0xCD, 0xc0c0c0); }
    for (int i = wx-41; i < wx-1; i++) { print16char(i, 0, 0xCD, 0xc0c0c0); print16char(i, wy-1, 0xCD, 0xc0c0c0); }

    // Вывод дампа регистров
    print(wx-38, 1, " 0  1  2  3  4  5  6  7  8  9", 0x00c000);

    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 10; j++) {

        int ai  = i*10 + j;
        print16char(wx-40, 2 + i, '0' + i, 0x00c000);

        // Допустимый регистр
        if (ai < 32) {

            sprintf(tmp, "%02X", sram[ai]);
            print(wx-38 + j*3, 2 + i, tmp, pvsram[ai] == sram[ai] ? 0xc0c0c0 : 0xffff80);
        }
    }

    // Дамп портов
    print(wx-25, 6, " 0  1  2  3  4  5  6  7", 0x00c000);
    for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++) {

        int ai = 32 + i*8 + j;
        sprintf(tmp, "%02X", sram[ai]);
        print(wx-25 + j*3, 7 + i, tmp, pvsram[ai] == sram[ai] ? 0xc0c0c0 : 0xffff80);

        sprintf(tmp, "%02X", i*8);
        print(wx-28, 7 + i, tmp, 0x00c000);
    }

    // Регистры
    sprintf(tmp, " X: %04X", get_X()); print(wx-40, 7, tmp, 0xc0c0c0);
    sprintf(tmp, " Y: %04X", get_Y()); print(wx-40, 8, tmp, 0xc0c0c0);
    sprintf(tmp, " Z: %04X", get_Z()); print(wx-40, 9, tmp, 0xc0c0c0);
    sprintf(tmp, "SP: %04X", get_S()); print(wx-40, 10, tmp, 0xc0c0c0);
    sprintf(tmp, "PC: %04X", pc);      print(wx-40, 11, tmp, 0xc0c0c0);
    sprintf(tmp, "%08X", instr_counter); print(wx-40, 12, tmp, 0x808080);

    // Флаги
    print16char(wx-40, 14, flag.i ? 'I' : 'i', flag.i ? 0x00ff00 : 0x808080);
    print16char(wx-39, 14, flag.t ? 'T' : 't', flag.t ? 0x00ff00 : 0x808080);
    print16char(wx-38, 14, flag.h ? 'H' : 'h', flag.h ? 0x00ff00 : 0x808080);
    print16char(wx-37, 14, flag.s ? 'S' : 's', flag.s ? 0x00ff00 : 0x808080);
    print16char(wx-36, 14, flag.v ? 'V' : 'v', flag.v ? 0x00ff00 : 0x808080);
    print16char(wx-35, 14, flag.n ? 'N' : 'n', flag.n ? 0x00ff00 : 0x808080);
    print16char(wx-34, 14, flag.z ? 'Z' : 'z', flag.z ? 0x00ff00 : 0x808080);
    print16char(wx-33, 14, flag.c ? 'C' : 'c', flag.c ? 0x00ff00 : 0x808080);

    // Память
    print(wx-35, 16, " 0 1 2 3 4 5 6 7 8 9 A B C D E F", 0x00c000);

    ds_dump_cursor &= 0xffff;
    ds_dump_start  &= 0xffff;

    // Количество строк ограничено нижним пределом
    for (int i = 0; i < (wy - 18); i++) {

        sprintf(tmp, "%04X", (ds_dump_start + 16*i) & 0xffff);

        cl = 0xf0f0c0;
        if (ds_tab == 1 && (ds_dump_cursor & 0xFFF0) == (ds_dump_start + 16*i))
            cl = 0x00ff00;

        // Печать текущего адреса
        print(wx-40, 17 + i, tmp, cl);

        // Печать дампа
        for (int j = 0; j < 16; j++) {

            int ai = ds_dump_start + ((16*i + j) & 0xffff);
            sprintf(tmp, "%02X", sram[ai]);

            print(wx-40 + 5 + 2*j, 17 + i, tmp, j % 2 ? 0xa0f0a0 : 0xa0a0a0);
        }
    }

    // Сохранить, чтобы вычислять изменения
    for (int i = 0; i < 96; i++) pvsram[i] = sram[i];
}
