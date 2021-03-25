#include <avrio.cc>
#include "kb.h"

class KB {

protected:

    byte _shift, _spec, _ctrl, _alt;

public:

    KB() {

        _shift = 0;
        _alt   = 0;
        _ctrl  = 0;
        _spec  = 0;
    }

    inline byte hit() { return inp(KB_HIT) & 1; }
    inline byte key() { return inp(KB_DATA); }

    // Чтение из порта нового символа
    int inkey() {

        int ascii = 0;

        // Ожидание нажатия клавиши
        if (hit()) {

            ascii = key();

                 if (ascii == key_LSHIFT || ascii == key_RSHIFT) { _shift = 1; ascii = 0; }
            else if (ascii == key_LCTRL)        { _ctrl  = 1; ascii = 0; }
            else if (ascii == key_LALT)         { _alt   = 1; ascii = 0; }
            else if (ascii == key_LCTRL +0x80)  { _ctrl  = 0; ascii = 0; }
            else if (ascii == key_LALT  +0x80)  { _alt   = 0; ascii = 0; }
            else if (ascii == key_LSHIFT+0x80)  { _shift = 0; ascii = 0; }
            else if (ascii == key_RSHIFT+0x80)  { _shift = 0; ascii = 0; }

            // Кнопку отпускания не использовать
            if ((ascii & 0x80)) ascii = 0;

            // Перевод в нижний регистр при отжатом SHIFT
            if (ascii >= 'A' && ascii <= 'Z' && _shift == 0) {
                ascii += ('a' - 'A');
            }
            // Поиск соответствий для возведения в верхний регистр
            else if (_shift && ascii) {

                for (int i = 0; i < KbShiftUpCount; i++) {
                    if (pgm_read_byte(&KbShiftUp[i][0]) == ascii) {
                        ascii = pgm_read_byte(&KbShiftUp[i][1]);
                        break;
                    }
                }
            }
        }

        return ascii;
    }

    // Ожидание получения нажатия клавиши
    int getch() { byte k; while ((k = inkey()) == 0); return k; }
};
