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
    SD_WriteError2      = 9,
    SD_WriteError3      = 10,
    SD_TimeoutErrorWRT  = 11,
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
    byte error_code;
    byte SD_type;

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

        long i;
        byte crc, status = 0xFF, on_error = 1;

        error_id = 0;

        // Включить устройство CE=0
        _command(SPI_CMD_CE0);

        // Принять байты, до тех пор, пока не будет 0xFF
        for (i = 0; i < 65536; i++) {
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
        for (i = 0; i < 256; i++)
            if (((status = get()) & 0x80) == 0)
                break;

        return status;
    }

    // Расширенная команда
    byte acmd(byte cmd, dword arg) {

        command(SD_CMD55, 0);
        return command(cmd, arg);
    }

    // Инициализация устройства
    byte init() {

        byte  status, i;
        dword arg;

        // Отослать команду `INIT`
        _command(SPI_CMD_INIT);

        // Тест на возможность войти в IDLE
        if (command(SD_CMD0, 0) != R1_IDLE_STATE) {
            return set_error(SD_UnknownError);
        }

        // Определить тип карты (SD1)
        if (command(SD_CMD8, 0x1AA) & R1_ILLEGAL_COMMAND) {

            // Есть illegal-бит, это карта SD1
            SD_type = SD_CARD_TYPE_SD1;

        } else {

            // Прочесть 4 байта, последний будет важный
            for (i = 0; i < 4; i++) status = get();

            // Неизвестный тип карты!
            if (status != 0xAA) return set_error(SD_UnknownCard);

            // Это тип карты SD2
            SD_type = SD_CARD_TYPE_SD2;
        }

        // Инициализация карты и отправка кода поддержки SDHC если SD2
        i   = 0;
        arg = (SD_type == SD_CARD_TYPE_SD2 ? 0x40000000 : 0);

        // Отсылка ACMD = 0x29. Отсылать и ждать, пока не придет корректный ответ
        while ((status = acmd(0x29, arg)) != R1_READY_STATE) {
            if (i++ > 8192) return set_error(SD_AcmdError);
        }

        // Если SD2, читать OCR регистр для проверки SDHC карты
        if (SD_type == SD_CARD_TYPE_SD2) {

            // Проверка наличия байта в ответе CMD58 (должно быть 0)
            if (command(SD_CMD58, 0)) {
                return set_error(SD_Unknown58CMD);
            }

            // Прочесть ответ от карты и определить тип (SDHC если есть)
            if ((get() & 0xC0) == 0xC0) {
                SD_type = SD_CARD_TYPE_SDHC;
            }

            // Удалить остатки от OCR
            for (i = 0; i < 3; i++) get();
        }

        return set_error(SD_OK);
    }

public:

    SD() { outp(SD_CMD, 0); init(); }

    byte sector[512];
    byte get_sd_type() { return SD_type; }
    byte get_status()  { return error_id; }
    byte get_error_code() { return error_code; }

    // Читать блок 512 байт в память: записывается результат в SD_data
    byte read(dword lba, byte SD_data[]) {

        int i = 0;
        byte status;

        set_error(SD_OK);

        // В случае истечения таймаута ожидания
        if (inp(STATUS) & SD_TIMEOUT)
            init();

        // Кроме SDHC ничего не поддерживается
        if (SD_type != SD_CARD_TYPE_SDHC)
            return set_error(SD_UnsupportYet);

        // Отослать команду поиска блока
        if (command(SD_CMD17, lba))
            return set_error(SD_BlockSearchError);

        // Ожидание ответа от SD
        while ((status = get()) == 0xFF)
            if (i++ > 8192)
                return set_error(SD_TimeoutError);

        // DATA_START_BLOCK = 0xFE
        if (status != 0xFE) {
            return set_error(SD_BlockSearchError);
        }

        // Прочесть данные
        for (i = 0; i < 512; i++) {
            SD_data[i] = get();
        }

        return set_error(SD_OK);
    }

    // Писать блок 512 байт в память
    byte write(dword lba, byte SD_data[]) {

        int i = 0;
        byte status;

        error_code = 0;
        set_error(SD_OK);

        // В случае истечения таймаута ожидания
        if (inp(STATUS) & SD_TIMEOUT)
            init();

        // Кроме SDHC ничего не поддерживается
        if (SD_type != SD_CARD_TYPE_SDHC)
            return set_error(SD_UnsupportYet);

        // Отослать команду поиска блока
        if (command(SD_CMD24, lba))
            return set_error(SD_BlockSearchError);

        // DATA_START_BLOCK
        put(0xFE);

        // Запись данных
        for (int i = 0; i < 512; i++) put(SD_data[i]);

        // Dummy 16-bit CRC
        put(0xFF);
        put(0xFF);

        status = get();

        // Сверить результат
        if ((status & 0x1F) != 0x05)
            return set_error(SD_WriteError);

        // Ожидание окончания программирования
        while ((status = get()) == 0xFF)
            if (++i >= 8192)
                return set_error(SD_TimeoutErrorWRT);

        // =========================
        // Тестирование после записи
        // =========================

        // Отсылка CMD13 (SEND_STATUS), должен быть 0
        if ((status = command(SD_CMD13, 0))) {
            error_code = status;
            return set_error(SD_WriteError2);
        }
        // Если 0, проверить что следующий байт тоже 0
        else if ((status = get())) {
            error_code = status;
            return set_error(SD_WriteError3);
        }

        return set_error(SD_OK);
    }

    // Чтение и запись во временную память
    byte read(dword lba)  { return read (lba, sector); }
    byte write(dword lba) { return write(lba, sector); }
};
