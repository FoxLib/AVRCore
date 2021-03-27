#ifndef __AVRIO_HEADER_FILE
#define __AVRIO_HEADER_FILE

// Ссылка на пустой адрес
#define NULL        ((void*)0)
#define brk         asm volatile("sleep"); // break

// Базовые типы данных
#define byte        unsigned char
#define word        unsigned int
#define dword       unsigned long

enum EnumStatuses {

    DRAM_WE         = 0x80,
    DRAM_BSY        = 0x40,

};

// Описания всех портов
enum EnumPORTSID {

    BANK            = 0x00, // RW
    KEYB            = 0x01, // R
    STATUS          = 0x02, // RW
    CURSOR_X        = 0x0C, // RW
    CURSOR_Y        = 0x0D, // RW
    TIMERL          = 0x0E, // R
    TIMERH          = 0x0F, // R
    DRAM0A          = 0x10, // RW
    DRAM1A          = 0x11, // RW
    DRAM2A          = 0x12, // RW
    DRAM3A          = 0x13, // RW
    DRAMD           = 0x14, // RW
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

enum KEY_ASCII {

    key_LSHIFT      = 0x01,
    key_LALT        = 0x02,
    key_LCTRL       = 0x03,
    key_UP          = 0x04,
    key_DN          = 0x05,
    key_LF          = 0x06,
    key_RT          = 0x07,
    key_BS          = 0x08,
    key_TAB         = 0x09,
    key_ENTER       = 0x0A,
    key_HOME        = 0x0B,
    key_END         = 0x0C,
    key_PGUP        = 0x0D,
    key_PGDN        = 0x0E,
    key_DEL         = 0x0F,
    key_F1          = 0x10,
    key_F2          = 0x11,
    key_F3          = 0x12,
    key_F4          = 0x13,
    key_F5          = 0x14,
    key_F6          = 0x15,
    key_F7          = 0x16,
    key_F8          = 0x17,
    key_F9          = 0x18,
    key_F10         = 0x19,
    key_F11         = 0x1A,
    key_ESC         = 0x1B,
    key_INS         = 0x1C,
    key_NL          = 0x1D,
    key_F12         = 0x1E,
    key_SPECIAL     = 0x1F          // Особая клавиша
};

// Чтение|запись в порты
inline byte inp(int port) { return ((volatile byte*)0x20)[port]; }
inline void outp(int port, unsigned char val) { ((volatile unsigned char*)0x20)[port] = val; }

// Объявление указателя на память (имя x, адрес a)
#define heap(x, a)  byte* x = (byte*) a
#define bank(x)     outp(BANK, x)
#define heapvm      byte* vm = (byte*) 0xf000

#endif
