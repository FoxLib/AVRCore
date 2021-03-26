#include <avrio.cc>

class keyboard {

protected:

    byte _phit;

public:

    keyboard() { _phit = inp(STATUS); }

    // Прочитать следующую клавишу
    byte getch() {

        byte status;

        do {

            // Ожидать смены клавиши
            while ((((status = inp(STATUS)) ^ _phit) & 1) == 0);

            _phit  = status;
            status = inp(KEYB);

        } while (status & 0x80 || status < 0x04);

        return status & 0x7F;
    }
};
