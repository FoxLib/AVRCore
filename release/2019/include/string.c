void strcpy(char* __dest, const char * __src);
void reverse(char s[]);
void itoa(int n, char s[]);
int  strlen(const char *__s);

// ---------------------------------------------------------------------

// Копирование строки
void strcpy(char* __dest, const char * __src) {

    int i = 0;
    while (__src[i]) {
        __dest[i] = __src[i];
        i++;
    }
    __dest[i] = 0;
}

// Длина строки
int strlen(const char *__s) {

    int i = 0;
    while (__s[i++]);
    return i;
}

// Переворачиваем строку s на месте
void reverse(char s[]) {

    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

// Конвертируем `n` в символы в s
void itoa(int n, char s[]) {

    int i, sign;

    // записываем знак
    // делаем n положительным числом
    if ((sign = n) < 0)
        n = -n;

    i = 0;

    /* генерируем цифры в обратном порядке */
    /* берем следующую цифру */
    do {
         s[i++] = n % 10 + '0';
    }
    /* удаляем */
    while ((n /= 10) > 0);

    if (sign < 0)
        s[i++] = '-';

    s[i] = '\0';
    reverse(s);
}
