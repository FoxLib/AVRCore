
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

    // Чтение из BIOS
    if (a >= 0xf0000) return pgm_read_byte(&BIOS[a & 0xffff]);

    // Видеопамять
    if (a >= 0xb8000 && a < 0xb9000)
        return vm[a & 0xfff];

    // Чтение из памяти
    return M.peek(a);
}

// Запись байта в память
void writememb(uint32_t a, uint8_t v) {

    heap(vm, 0xf000);

    // Обнаружена запись в видеопамять
    if (a >= 0xb8000 && a < 0xb9000)
        vm[a & 0xfff] = v;

    M.poke(a, v);
}

