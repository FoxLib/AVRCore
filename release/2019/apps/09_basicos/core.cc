#include "core.h"

class TCore {

protected:

    byte    process_count;
    byte    process_current;
    struct  TProcess process[16];

    byte    tbuf[80], tb_size, tb_cursor;
    byte    term_x, term_y, term_color;

public:

    // Инициализация
    TCore() {

        // Главный процесс
        process[0].pid = 1;
        strcpy(process[0].name, "Terminal");

        process_current = 0;
        process_count   = 1;

        // Курсор в терминале
        term_x     = 1;
        term_y     = 22;
        term_color = 0x07;
    }

    // Обновление панели
    void update_panel(byte active) {

        t.hide();
        t.block(0, 24, 79, 24, 0x70);
        t.cursor(1, 24);

        // Печать имен процессов
        for (int i = 0; i < process_count; i++) {

            t.color(process_current == i ? (active ? 0x2F : 0x8F) : 0x70);
            t.printch(' ');
            t.print(process[i].name);
            t.printch(' ');
        }
    }

    // Полное обновление
    void term_update() {

        t.block(0, 0, 79, 23, 0x07);
        t.frame(0, 0, 79, 23, 0);
        t.cursor(1, 0);
        t.color(0x0F);
        t.putf8(" Терминал ");

        // Обновить сохраненный контент

        t.cursor(1, 1);
        t.show();
    }

    // Перемотка вверх
    void term_scrollup() {

        heap(vm, 0xF000);

        // Перемотка
        for (int i = 1; i < 20; i++) {

            int k = i*160;
            for (int j = 1; j < 78; j++) {
            }
        }
    }

    // Записать символ в терминал
    void term_put(byte ch) {

        // Клавиша ENTER
        if (ch == 10) {

            term_x = 1;
            term_y++;
        }
        // Следующий символ
        else {

            t.color(term_color);
            t.printc(term_x, term_y, ch);

            term_x++;
            if (term_x > 78) {
                term_x = 1;
                term_y++;
            }
        }

        // Перемотка наверх при достижении нижней части экрана
        if (term_y >= 22) {

            term_y = 22;
            term_scrollup();
        }

        t.cursor(term_x, term_y);
    }

    // Перерисовать строку
    void term_update_row() {

        t.hide();
        t.cursor(3, term_y);
        t.print((const char*) tbuf);
        t.cursor(3 + tb_cursor, term_y);
        t.show();
    }

    // Ввод команды в буфер (не более 1 строки)
    int term_input() {

        tb_size = 0;
        tb_cursor = 0;

        t.cursor(1, term_y);
        t.color(0x07);
        t.print("> ");

        for (;;) {

            byte ch = kb.getch();

            // Удаление предыдущего символа
            if (ch == VK_BACKSPACE) {

                if (tb_cursor > 0) {

                    tb_cursor--;

                    // Удалить символ перед курсором
                    for (int i = tb_cursor; i < tb_size; i++)
                        tbuf[i] = tbuf[i + 1];

                    tbuf[--tb_size] = 0;
                    t.printc(3 + tb_size, term_y, ' ');
                }
            }
            // Удаление символа из строки за курсором
            else if (ch == VK_DEL) {

                if (tb_cursor < tb_size) {

                    t.printc(2 + tb_size, term_y, ' ');
                    for (int i = tb_cursor; i < tb_size; i++)
                        tbuf[i] = tbuf[i + 1];

                    tbuf[--tb_size] = 0;
                }
            }
            // Клавиша влево
            else if (ch == VK_LEFT) {

                if (tb_cursor > 0)
                    tb_cursor--;
            }
            // Клавиша вправо
            else if (ch == VK_RIGHT) {

                if (tb_cursor < tb_size)
                    tb_cursor++;
            }
            // Вставка нового символа в буфер (если возможно)
            else if (tb_size < 75) {

                for (int i = tb_size; i > tb_cursor; i--)
                    tbuf[i] = tbuf[i-1];

                tbuf[tb_cursor] = ch;
                tbuf[++tb_size] = '\0';
                tb_cursor++;
            }

            // Команда принята
            if (ch == 10) {

                term_put(ch);
                return 1;
            }

            term_update_row();
        }

        // Переход к другому процессу
        return 0;
    }

    // Окно терминала
    void term_process() {

        term_update();

        for(;;) {

            // Выполнение команды
            if (term_input()) {



            }
            // Выход к навигации
            else break;
        }
    }

    // Основной цикл ОС
    void main() {

        update_panel(0);

        for (;;) {

            // Если текущий процесс нулевой, мы попадает в терминал
            if (process_current == 0) term_process();

            // Иначе исполняется инструкция байт-кода
        }
    }
};
