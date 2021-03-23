#include "sokoban.h"

class AppSokoban {

protected:

    int   player_x, player_y;
    byte  freecnt, current_level;
    int   steps, realstep;
    dword current_time, start_time;
    byte  dirmario;

public:

    void main() {

        g.start();

        current_level = 0;

        byte lvl[240];
        byte ch, ob, exit = 0;

        char movex, movey, moved;

        while (exit == 0) {

            current_time  = 0;
            dirmario      = 0;
            start_time    = (TIMERD / 1000);

            load_level(lvl);
            draw_level(lvl);

            for (;;) {

                movex = 0;
                movey = 0;
                moved = 0;

                // Нарисовать игрока
                sprite(player_x, player_y, SOKOBAN_PLAYER);
                show_stat(lvl);

                // Победа
                if (freecnt == 0) { you_win(); break; }

                // Вращение времени
                while ((ch = kb.inkey()) == 0) {
                    show_timer();
                }

                if      (ch == VK_UP)    { movex =  0; movey = -1; }
                else if (ch == VK_DOWN)  { movex =  0; movey =  1; }
                else if (ch == VK_LEFT)  { movex = -1; movey =  0; dirmario = 1; }
                else if (ch == VK_RIGHT) { movex =  1; movey =  0; dirmario = 0; }
                else if (ch == VK_ESC)   { exit = 1; break; }
                else if (ch == VK_ENTER) { exit = 0; break; }
                else if (ch == VK_TAB)   { exit = 0; level_select(); break; }

                // Зарегистрировано перещение
                if (movex || movey) {

                    // Проверка, куда перемещается
                    ob = get_obj_hi(player_x + movex, player_y + movey, lvl);

                    if (ob == SOKOBAN_BRICK) {
                        // Загораживает стена
                    }
                    else if (ob == SOKOBAN_BOX) {

                        ob = get_obj_hi(player_x + 2*movex, player_y + 2*movey, lvl);

                        if (ob == SOKOBAN_BOX || ob == SOKOBAN_BRICK) {
                            // За ящиком стена или ящик
                        }
                        else {

                            moved = 1;

                            // Очистить коробку на прежнем месте и передвинуть
                            lvl[20*(player_y + 1*movey) + (player_x + 1*movex)] &= 0xF0;
                            lvl[20*(player_y + 2*movey) + (player_x + 2*movex)] |= SOKOBAN_BOX;

                            update_sprite(player_x + 1*movex, player_y + 1*movey, lvl);
                            update_sprite(player_x + 2*movex, player_y + 2*movey, lvl);

                            steps++;
                        }
                    }
                    else moved = 1;
                }

                // Перемещение доступно
                if (moved) {

                    // Нарисовать задний фон у игрока
                    update_player(lvl);

                    player_x += movex;
                    player_y += movey;

                    realstep++;
                }
            }
        }
    }

    // Вы прошли уровень
    void you_win() {

        for (int i = 0; i < 200; i++)
        for (int j = i&1; j < 320; j += 2)
            g.pset(j, i, 0);

        g.wiped(0)->window(50, 50, 200, 40, "Вы прошли уровень");
        g.cursor(54, 70)->color(0)->print("Перейти к следующему...");

        current_level++;

        if (current_level == SOKOBAN_MAX_LEVEL)
            current_level = 0;

        kb.getch();
    }

    // Выбор уровня
    void level_select() {

        byte ch;
        g.wiped(0)->window(50, 50, 200, 40, "Выбор уровня");
        g.wiped(0x17)->color(0);

        for (;;) {

            g.cursor(54, 72);
            g.print("Уровень ");
            g.printint(1 + current_level);
            g.print("  ");

            // Выбор уровня
            ch = kb.getch();

            if (ch == VK_DOWN) current_level--;
            else if (ch == VK_UP) current_level++;
            else if (ch == VK_ENTER || ch == VK_ESC) break;

            // Ограничитель
            if (current_level < 0) current_level = SOKOBAN_MAX_LEVEL - 1;
            else if (current_level >= SOKOBAN_MAX_LEVEL) current_level = 0;
        }
    }

    // Кол-во остатков
    byte count_free(byte lvl[]) {

        byte cnt = 0;

        for (int i = 0; i < 12; i++)
        for (int j = 0; j < 20; j++) {

            byte hi = lvl[20*i + j] >> 4;
            byte lo = lvl[20*i + j] & 0x0F;

            if (hi == SOKOBAN_PLACE && lo != SOKOBAN_BOX)
                cnt++;
        }

        return cnt;
    }

    // Показать статистику
    void show_stat(byte lvl[]) {

        freecnt = count_free(lvl);

        g.cursor(0, 0)->color(15)->wiped(0x10);
        g.print("#");
        g.printint(1 + current_level);
        g.print(" Ящик ");
        g.printint(freecnt);
        g.print("  Ход ");
        g.printint(steps);
        g.print("  Шаг ");
        g.printint(realstep);
        g.print("  ");

    }

    // Показать таймер
    void show_timer() {

        dword ctime = TIMERD / 1000;

        if (ctime != current_time) {
            ctime -= start_time;

            g.wiped(0x10)->color(15)->cursor(260, 0);

            byte sec = ctime % 60;
            byte min = ctime / 60;
            byte hrs = min / 60;
            min %= 60;

            g.printint(hrs);
            g.printch(':');
            if (min < 10) g.printch('0'); g.printint(min);
            g.printch(':');
            if (sec < 10) g.printch('0'); g.printint(sec);
        }
    }

    // Объект, который стоит на уровне игрока
    byte get_obj_hi(int x, int y, byte lvl[]) {
        return lvl[20*y + x] & 0x0F;
    }

    // Обновить спрайт
    void update_sprite(int x, int y, byte lvl[]) {

        byte hi = lvl[20*y + x] >> 4;
        byte lo = lvl[20*y + x] & 0x0F;

        // Гарисовать Place, если нет ничего (например ящика)
        if (lo == 0 && hi == SOKOBAN_PLACE)
             sprite(x, y, hi);
        else sprite(x, y, lo);
    }

    // Чтобы поменьше писать
    void update_player(byte lvl[]) { update_sprite(player_x, player_y, lvl); }

    // Загрузка уровня в память
    void load_level(byte store[]) {

        steps = 0;
        realstep = 0;

        for (int i = 0; i < 12; i++) {

            for (int j = 0; j < 20; j++) {

                byte vb = sokoban_level[current_level][ (j>>2) + (i*5) ];
                byte sp = (vb >> (6 - 2*(j & 3))) & 3;

                store[20*i + j] = (sp == SOKOBAN_PLACE) ? (SOKOBAN_PLACE << 4) : sp;
            }
        }

        // Записать позицию игрока
        player_x = sokoban_level[current_level][60];
        player_y = sokoban_level[current_level][61];
    }

    // Нарисовать уровень и загрузить его в память
    void draw_level(byte level[]) { // byte lvl

        g.cls(0);
        for (int i = 0; i < 12; i++)
        for (int j = 0; j < 20; j++) {
            update_sprite(j, i, level);
        }
    }

    // Нарисовать некоторый спрайт
    void sprite(int x, int y, int id) {

        char i, j, k = 0;
        byte cl;

        x *= 16;
        y *= 16;

        switch (id) {

            // Пустота
            case 0:

                g.block(x, y, x+15, y+15, 0);
                break;

            // Кирпичи
            case SOKOBAN_BRICK:

                g.block(x, y, x+15, y+15, 3);
                for (i = 0; i < 16; i += 4) {
                    g.line(x,y+i,x+15,y+i,0);
                    for (j = k&1?3:7; j < 16; j += 8)
                        g.line(x+j,y+i,x+j,y+i+3,0);
                    k++;
                }
                break;

            // Доски
            case SOKOBAN_WOOD:
            case SOKOBAN_PLACE:

                g.block(x, y, x+15, y+15, 4);
                for (i = 0; i < 16; i += 4) {
                    g.line(x,y+i,x+15,y+i,0);
                }

                // 2 доски
                g.line(x+15,y,x+15,y+3,0);
                g.line(x+15,y+8,x+15,y+11,0);

                // 3 гвоздя
                g.pset(x+2,y+2,0);
                g.pset(x+2,y+10,0);
                g.pset(x+12,y+6,0);
                g.pset(x+12,y+14,0);

                // Это место для парковки
                if (id == SOKOBAN_PLACE) g.circle(x+8,y+8,3,7);

                break;

            // Коробка для сокобана
            case SOKOBAN_BOX:

                g.block(x, y, x+15, y+15, 6);
                g.lineb(x, y, x+15, y+3, 0);
                g.lineb(x, y+3, x+3, y+12, 0);
                g.lineb(x+12, y+3, x+15, y+12, 0);
                g.lineb(x, y+12, x+15, y+15, 0);
                g.line (x+3, y+6, x+12, y+6, 0);
                g.line (x+3, y+9, x+12, y+9, 0);
                break;

            // Рисование марио
            case SOKOBAN_PLAYER:

                for (i = 0; i < 16; i++)
                for (j = 0; j < 16; j++) {

                    k = mario[i*4 + (j>>2)];
                    cl = (k >> (6 - 2*(j&3))) & 3;

                    switch (cl) {

                        case 1: cl = 6; break;
                        case 2: cl = 4; break;
                        case 3: cl = 7; break;
                    }

                    if (cl > 0) g.pset(x + (dirmario ? 16 - j : j), y + i, cl);
                }

                break;
        }
    }
};
