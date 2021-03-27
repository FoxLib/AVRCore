#include "avrio.cc"

enum SD_Commands {

    SD_CMD0     = 0,    // Сброс
    SD_CMD8     = 8,    // Проверка вольтажа SD2
    SD_CMD13    = 13,   // Проверка
    SD_CMD17    = 17,   // Чтение
    SD_CMD24    = 24,   // Запись
    SD_CMD55    = 55,   // ACMD
    SD_CMD58    = 58    // Чтение регистра OCR
};

enum SD_Results {

    R1_READY_STATE      = 0x00,
    R1_IDLE_STATE       = 0x01,
    R1_ILLEGAL_COMMAND  = 0x04
};

enum SD_Errors {

    SD_OK               = 0,
    SD_UnknownError     = 1,
    SD_TimeoutError     = 2,
    SD_UnknownCard      = 3,
    SD_AcmdError        = 4,
    SD_Unknown58CMD     = 5,
    SD_BlockSearchError = 6,
    SD_UnsupportYet     = 7,
    SD_WriteError       = 8,
    SD_WriteError2      = 9
};

enum EnumSPICommands {

    SPI_CMD_INIT    = 0,
    SPI_CMD_SEND    = 1,
    SPI_CMD_CE0     = 2,
    SPI_CMD_CE1     = 3
};

enum SD_Types {

    SD_CARD_TYPE_ERR    = 255,
    SD_CARD_TYPE_SD1    = 1,
    SD_CARD_TYPE_SD2    = 2,
    SD_CARD_TYPE_SDHC   = 3
};


class SD {
protected:

    byte error_id;

    // Процедура отсылки команды на SD-контроллер
    void _command(byte data) {

        while (inp(STATUS) & SD_BUSY);      // Ожидать пока освободится
        outp(SD_CMD, data | 0x80);          // Защелкнуть
        while (inp(STATUS) & SD_BUSY);      // Ожидать освобождения
        outp(SD_CMD, inp(SD_CMD) & ~0x80);  // Отщелкнуть
    }

    // Отсылка байта к SD
    void put(byte data) {

        outp(SD_DATA, data);
        _command(SPI_CMD_SEND);
    }

    // Получение байта от SD
    byte get() {

        put(0xff);
        return inp(SD_DATA);
    }

    // Установка ошибки
    byte set_error(byte _error_id) {

        error_id = _error_id;
        _command(SPI_CMD_CE1);
        return _error_id;
    }

    // Отсылка команды на SD-карту
    byte command(byte cmd, dword arg) {

        int i;
        byte crc, status = 0xFF, on_error = 1;

        error_id = 0;

        // Включить устройство CE=0
        _command(SPI_CMD_CE0);

        // Принять байты, до тех пор, пока не будет 0xFF
        for (i = 0; i < 4095; i++) {
            if (get() == 0xFF) {
                on_error = 0; break;
            }
        }

        // Ошибка ожидания ответа от SPI
        if (on_error) return set_error(SD_TimeoutError);

        // Отсылка команды к SD
        put(cmd | 0x40);

        // Отослать 32-х битную команду
        for (i = 24; i >= 0; i -= 8) put(arg >> i);

        // Отправка CRC
        if (cmd == SD_CMD0) crc = 0x95;  // CMD0 with arg 0
        if (cmd == SD_CMD8) crc = 0x87;  // CMD8 with arg 0x1AA

        put(crc);

        // Ожидать снятия флага BUSY
        for (i = 0; i < 255; i++)
            if (((status = get()) & 0x80) == 0)
                break;

        return status;
    }

public:

    SD() { outp(SD_CMD, 0); }

    // Инициализация устройства
    byte init() {

        // Отослать команду `INIT`
        _command(SPI_CMD_INIT);

        // Тест на возможность войти в IDLE
        if (command(SD_CMD0, 0) != R1_IDLE_STATE) {
            return set_error(SD_UnknownError);
        }

        return SD_OK;
    }
};
