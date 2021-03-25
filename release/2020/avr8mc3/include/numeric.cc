#ifndef __FILE_NUMERIC
#define __FILE_NUMERIC

class Numeric {

protected:

    char buf[24];

public:

    // Печать числа -2147483647 .. 2147483647
    byte i2a(long v) {

        char s[24];
        int  q, i = 0, cnt = 0, cs = 0;

        // Печать символа минус перед числом
        if (v < 0) { v = -v; buf[cs++] = '-'; cnt = 1; }

        // Вычисление смещения
        do { q = v % 10; v /= 10; s[i++] = '0' + q; } while (v); i--;

        // Вывести число
        for (char k = 0; k <= i; k++) { buf[cs++] = s[i-k]; cnt++; }


        buf[cs] = 0;    // В конце ZTerm
        return cnt;     // Занимаемый размер символов
    }

    // Печать float, n=2
    byte f2a(float x, char n) {

        byte tmpbuf[32];
        byte cs = 0, t = 0;

        // Отрицательное число
        if (x < 0) { x = -x; tmpbuf[cs++] = '-'; }

        // Получить целочисленную и дробную части
        unsigned long i = (unsigned long) x;
        float f = x - i;

        // Печать целочисленного значения
        i2a(i);
        while (buf[t]) tmpbuf[cs++] = buf[t++];
        tmpbuf[cs++] = '.';

        // Печать остатка
        for (int k = 0; k < n; k++) {

            f *= 10;
            tmpbuf[cs++] = f + '0';
            f = f - (int)f;
        }

        // Скопировать во временный буфер
        for (t = 0; t < cs; t++) buf[t] = tmpbuf[t];
        buf[cs] = 0;

        return cs;
    }
};

#endif
