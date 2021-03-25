#include <avr/pgmspace.h>

//    тип     имя        тип памяти      данные
const uint8_t pgm_data[] PROGMEM = "Program Memory Space";

// Шаблон с чтением из PGM
int main() {

    int i = 0;

    // Указатель на видеопамять
    uint8_t* vm = (uint8_t*) 0x8000;

    // Извлечь и скопировать в память самостоятельно
    while (uint8_t ch = pgm_read_byte(&pgm_data[i])) {
        vm[i++] = ch;
    }

    // Бесконечный цикл
    for(;;);
}
