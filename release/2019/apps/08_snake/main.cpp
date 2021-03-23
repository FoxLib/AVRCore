#include <graphics.cpp>
#include <kb.cpp>

// --------------------
KB       kb;
Graphics g;
// --------------------

class snake {
    
protected:

    int  map[1000];
    int  size;          // Длина змеи
    char head_x, head_y, dirx, diry;
    word timer;
    
public:

    // Запуск программы
    void start() {

        g.start();
        g.cls(1);

        // Клеточки
        for (int i = 0; i < 1000; i++) map[i] = 0;
        for (int i = 0; i < 320; i += 8) g.line(i, 0, i, 199, 9);
        for (int i = 0; i < 200; i += 8) g.line(0, i, 319, i, 9);

        size   = 1;
        head_x = 20; head_y = 10;
        dirx   = 1;  diry   = 0;

        timer  = TIMERW;
    }

    // Засеивание едой
    void foodseed() {

        word k = 0;
        for (int i = 0; i < 16; i++) {
            
            k = ((k*12623) + 123) % 1000;
            map[k] = -1;
            drawcell(k % 40, k / 40, 10);        
        }
    }

    // Основной цикл
    void loop() {

        for (;;) {

            nextstep(); 
            byte ch = waithit();

            switch (ch) {

                case 1: dirx =  0; diry = -1; break;
                case 2: dirx =  1; diry =  0; break;
                case 3: dirx =  0; diry =  1; break;
                case 4: dirx = -1; diry =  0; break;
            }
        }
    }

    // Передвинуть
    void move() {

        int k = 0;

        // Обновить
        for (int y = 0; y < 25; y++)
        for (int x = 0; x < 40; x++) {

            // Хвост
            if (map[k] == 1) {

                map[k] = 0;
                drawcell(x, y, 1);
            }
            // Туловище
            else if (map[k] > 1) {
                map[k]--;
            }
            
            k++;
        }
    }

    // Обновить весь экран
    void nextstep() {

        
        // Следующая позиция головы
        head_x += dirx;
        head_y += diry;

        // Ошибка - выход за границы
        if (head_x >= 40 || head_x < 0 || head_y >= 25 || head_y < 0)
            the_end();

        // Новая позиция головы
        int cp = 40*head_y + head_x;

        // Башкой пробил себя
        if (map[cp] > 0) the_end();

        // Сожрал еду
        if (map[cp] < 0) size++; else move();            

        // Установить голову в новое положение
        map[cp] = size;

        // Нарисовать новую позицию
        drawcell(head_x, head_y, 7);        
    }

    // Игра окончена
    void the_end() {

        g.cursor(110, 92);
        g.print(" GAME OVER ");
        for(;;);
    }

    // Отлов нажатия клавиш
    byte waithit() {

        byte hit = 0;
        while(TIMERW - timer < 250) {

            byte ch = kb.inkey();
            if (ch) hit = ch;
        };

        timer = TIMERW;        
        return hit;
    }

    // Обновить ячейку
    void drawcell(char x, char y, char cl) {
        g.block(8*x + 1, 8*y + 1, 8*x + 7, 8*y + 7, cl);
    }
};

snake game;

int main() {

    game.start();
    game.foodseed();
    game.loop();
}
