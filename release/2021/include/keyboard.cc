#include <avrio.cc>

class keyboard {
protected:
public:

    // Прочитать следующую клавишу
    byte getch() {

        byte kb;

        do {

            // Отследить появление kbhit
            while ((inp(STATUS) & 1) == 0);

            kb = inp(KEYB);

            // Отослать сигнал для сброса kbhit
            outp(STATUS, inp(STATUS) | 1);

        } while (kb & 0x80 || kb < 0x04);

        return kb & 0x7F;
    }

    byte get() { return inp(KEYB); }
};
