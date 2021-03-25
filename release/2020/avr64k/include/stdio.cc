#include "avrio.cc"
#include "stdio.h"

class stdio {

protected:

    byte  _key_cnt;
    byte  _shift_key;
    dword _seed;

public:

    stdio() { _key_cnt = inp(KB_HIT); _shift_key = 0; _seed = 1; }

    // Значение таймера
    word timer() { return (inp(TIMER_LO) | inp(TIMER_HI)<<8); }

    // Захват клавиши на лету
    byte inkey() {

        byte kbd, kbh;

        for (;;) {

            // Есть нажатие на клавишу
            if (_key_cnt != (kbh = inp(KB_HIT))) {

                // Зафиксировать
                _key_cnt = kbh;
                 kbd     = inp(KB_DATA);

                // Специальные клавиши
                if      (kbd == key_LSHIFT)        _shift_key = 1;
                else if (kbd == key_LSHIFT + 0x80) _shift_key = 0;
                // Клавиша нажата
                else if ((kbd & 0x80) == 0) {

                    // Перевод символов в нижний регистр
                    if (!_shift_key && kbd >= 'A' && kbd <= 'Z') {
                        kbd += ('a' - 'A');
                    }
                    // Проверка на перевод регистра
                    else if (_shift_key) {

                        for (int i = 0; i < KB_SHIFT_TRAN_NUM; i++) {

                            // Проверить, есть ли такой символ в таблице перевода
                            if (pgm_read_byte(&kb_shift_trn[2*i]) == kbd) {
                                return pgm_read_byte(&kb_shift_trn[2*i + 1]);
                            }
                        }
                    }

                    return kbd;
                }
                // Неподходящая клавиша
                else break;
            }
            // Не обнаружено нажатых клавиш
            else break;
        }

        return 0;
    }

    // Ожидание нажатия клавиши
    byte getch() {

        byte k;
        while ((k = inkey()) == 0);
        return k;
    }

    void seed(dword seed) { _seed = seed; }

    // "Случайное" число
    dword rand() {

        _seed = (106    *_seed + 1283) % 6075;
        _seed = _seed ? _seed : 1;

        return _seed;
    }
};
