#include "sd.cc"

struct FAT_partition {
    byte  type;
    dword lba, size;
};

struct FAT_info {

};

class FAT : public SD {
protected:

    SD sd;

public:

    struct FAT_partition partitions[4];

    // Разобрать информацию о структуре разделов
    FAT() {

        sd.read(0);

        for (int i = 0; i < 4; i++) {

            // Получение указателя на память
            byte* t = &sd.sector[i*16 + 0x1be];

            // Считывание структуры
            partitions[i].type = t[4];
            partitions[i].lba  = ((dword*)(t+8))[0];
            partitions[i].size = ((dword*)(t+12))[0];
        }
    }

    // Получение основной информации о FAT-таблице
    struct FAT_info get_fat_info(int pid) {

        struct FAT_info t;

        return t;
    }
};
