#ifndef __AVRIO_HFILE
#define __AVRIO_HFILE

#include <avr/pgmspace.h>

// Ссылка на пустой адрес
#define NULL    ((void*) 0)
#define brk     asm volatile("sleep"); // break

// Базовые типы данных
#define byte    unsigned char
#define uint    unsigned int
#define word    unsigned int
#define ulong   unsigned long
#define dword   unsigned long

// Объявление указателя на память (имя x, адрес a)
#define heap(x, a)  byte* x = (byte*) a
#define bank(x)     outp(BANK_LO, x)

// Описания всех портов
enum InputOutputPort {

    // Банкинг
    BANK_LO         = 0x00, // RW
    BANK_HI         = 0x01, // RW

    // Клавиатура
    KB_DATA         = 0x02, // R
    KB_HIT          = 0x03, // R

    // Текстовый курсор
    CURSOR_X        = 0x04, // RW
    CURSOR_Y        = 0x05, // RW

    // Конфигурация таймера
    TIMER_LO        = 0x06, // R
    TIMER_HI        = 0x07, // R
    TIMER_HI2       = 0x0F, // R
    TIMER_HI3       = 0x10, // R
    TIMER_INTR      = 0x16, // W Прерывание таймера

    // SPI SD
    SPI_DATA        = 0x08, // W
    SPI_CMD         = 0x09, // W
    SPI_STATUS      = 0x09, // R

    // Эмулятор мыши
    MOUSE_X_LO      = 0x0A, // R
    MOUSE_Y_LO      = 0x0B, // R
    MOUSE_STATUS    = 0x0C, // R
    MOUSE_X_HI      = 0x0E, // R

    // Видеорежим
    VIDEOMODE       = 0x0D, // RW

    // Управление SDRAM
    SDRAM_B0        = 0x10, //  7:0
    SDRAM_B1        = 0x11, // 15:8
    SDRAM_B2        = 0x12, // 23:16
    SDRAM_B3        = 0x13, // 31:24
    SDRAM_DATA      = 0x14, // RW
    SDRAM_CTRL      = 0x15, // R  Status [0=Ready], W Control [0=WE]
};

// Список видеорежимов
enum VideoModes {

    VM_80x25        = 0,
    VM_320x200x8    = 1,
    VM_320x240x2    = 2,
    VM_320x200x4    = 3
};

// Чтение из порта
inline byte inp(int port) { return ((volatile byte*)0x20)[port]; }

// Запись в порт
inline void outp(int port, unsigned char val) { ((volatile unsigned char*)0x20)[port] = val; }

#endif
