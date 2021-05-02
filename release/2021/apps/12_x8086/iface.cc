
/**
 * Распределение памяти:
 *
 * 00000-003FF  IVT  1k
 * 00400-9FFFF  RAM  640k
 * B8000-B8FFF  TXT  4k
 * F0000-FFFFF  BIOS
 */

// Работа с памятью (чтение из BIOS/памяти)
uint8_t readmemb(uint32_t a) {

    heap(vm, 0xf000);

    // Чтение из BIOS 64k
    if (a >= 0xf0000)
        return pgm_read_byte(&BIOS[a & 0xffff]);

    // Видеопамять
    if (a >= 0xb8000 && a < 0xb9000)
        return vm[a & 0xfff];

    // Чтение из общей памяти
    return M.peek(a);
}

// Запись байта в память
void writememb(uint32_t a, uint8_t v) {

    heap(vm, 0xf000);

    // Обнаружена запись в видеопамять
    if (a >= 0xb8000 && a < 0xb9000) {
        vm[a & 0xfff] = v;
        return;
    }

    if (a >= 0xc0000)
        return;

    M.poke(a, v);
}

uint8_t ioread(uint16_t port) {
    return 0xff;
}

void iowrite(uint16_t port, uint8_t data) {
}
