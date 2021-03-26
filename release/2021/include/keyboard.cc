#include <avrio.cc>

class keyboard {
protected:
public:

    // Прочитать следующую клавишу
    byte getch() {

        byte kb;

        do {

            while ((inp(STATUS) & 0x10) == 0);
            kb = inp(KEYB);
            outp(STATUS, 0x10);

        } while (kb & 0x80 || kb < 0x04);

        return kb & 0x7F;
    }
};
