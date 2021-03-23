#include <avr/interrupt.h>

#include <textmode.cpp>
#include <graphics.cpp>
#include <kb.cpp>
#include <dram.cpp>

KB       kb;
DRAM     dram;
TextMode t;
Graphics g;

// Глобальный указатель на видеопамять в текстовом режиме
volatile char* m = (char*)0xF000;

// Назначение обработчка
ISR(INT0_vect) {
    m[434]++;
}

int main() {

    t.start();      // Запуск текстового режима
    t.cls(0x17);    // Экран в синий цвет и серые буквы

    // Интервал прерывания каждые 100 миллисекунд
    outp(TIMER_INTR, 100);

    // Включить прерывания
    sei();

    // Бесконечный цикл
    for(;;);
}
