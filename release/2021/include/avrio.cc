#ifndef __AVRIO_HEADER_FILE
#define __AVRIO_HEADER_FILE

// Ссылка на пустой адрес
#define NULL        ((void*)0)
#define brk         asm volatile("sleep"); // break

// Базовые типы данных
#define byte        unsigned char
#define word        unsigned int
#define dword       unsigned long

// Описания всех портов
enum EnumPORTSID {

    BANK            = 0x00, // RW
    KEYB            = 0x01, // R
    STATUS          = 0x02, // RW
    CURSOR_X        = 0x0C, // RW
    CURSOR_Y        = 0x0D, // RW
    TIMERL          = 0x0E, // R
    TIMERH          = 0x0F, // R
    VIDEOMODE       = 0x18, // RW
};

// Список видеорежимов
enum EnumVIDEOMODES {

    VIDEOMODE_80x25       = 0,
    VIDEOMODE_320x200x8_0 = 2,
    VIDEOMODE_320x200x8_1 = 3,
};

enum EnumSPICommands {

    SPI_CMD_INIT    = 0,
    SPI_CMD_SEND    = 1,
    SPI_CMD_CE0     = 2,
    SPI_CMD_CE1     = 3
};

// Чтение|запись в порты
inline byte inp(int port) { return ((volatile byte*)0x20)[port]; }
inline void outp(int port, unsigned char val) { ((volatile unsigned char*)0x20)[port] = val; }

// Объявление указателя на память (имя x, адрес a)
#define heap(x, a)  byte* x = (byte*) a
#define bank(x)     outp(BANK, x)

#endif
