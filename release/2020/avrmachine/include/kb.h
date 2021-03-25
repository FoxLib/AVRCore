#define KbShiftUpCount 21

const byte KbShiftUp[KbShiftUpCount][2] PROGMEM = {
    {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'},
    {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8', '*'},
    {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'},
    {'`', '~'}, {'[', '{'},  {']', '}'}, {'\'', '"'},
    {';', ':'}, {'\\', '|'}, {',', '<'}, {'.', '>'},
    {'/', '?'},
};

enum KBASCII {

    // Физические кнопки
    key_F1          = 0x01,
    key_F2          = 0x02,
    key_F3          = 0x03,
    key_F4          = 0x04,
    key_F5          = 0x05,
    key_F6          = 0x06,
    key_F7          = 0x07,
    key_BS          = 0x08,
    key_TAB         = 0x09,
    key_F8          = 0x0A,
    key_F9          = 0x0B,
    key_F10         = 0x0C,
    key_ENTER       = 0x0D,
    key_F11         = 0x0E,
    key_F12         = 0x0F,

    key_CAP         = 0x10,     // Caps Shift
    key_LSHIFT      = 0x11,
    key_LCTRL       = 0x12,
    key_LALT        = 0x13,
    key_LWIN        = 0x14,
    key_RSHIFT      = 0x15,
    key_RWIN        = 0x16,
    key_MENU        = 0x17,     // Кнопка Меню
    key_SCL         = 0x18,     // Scroll Lock
    key_NUM         = 0x19,     // Num Pad
    key_ESC         = 0x1B,     // Escape

    // Виртуальные
    key_UP          = -1,
    key_DN          = -2,
    key_LF          = -3,
    key_RT          = -4,
    key_HOME        = -5,
    key_END         = -6,
    key_PGUP        = -7,
    key_PGDN        = -8,
    key_DEL         = -9,
};
