<?php

// php createfb.php > memtext.mif

$mem = [];

// Разбор файла
$arr = file("cp866.txt");
$arr = array_map(function($e) {
    if (preg_match('~\{(.+)\}~', $e, $c)) {
        return preg_split('~,\s*~', $c[1]);
    }
    return trim($e);
}, $arr);

// Содержание видеопамяти
for ($i = 0; $i < 4000; $i += 2) {
    $mem[$i]   = 0x21;
    $mem[$i+1] = 0x07;
}

$palette = [
    0x111, 0x008, 0x080, 0x088,
    0x800, 0x808, 0x880, 0xccc,
    0x888, 0x00f, 0x0f0, 0x0ff,
    0xf00, 0xf0f, 0xff0, 0xfff,
];

// Палитра
for ($i = 0; $i < 16; $i++) {

    $mem[4000 + $i*2]     = $palette[$i] & 255;
    $mem[4000 + $i*2 + 1] = $palette[$i] >> 8;
}

// Знакогенератор
$address = 4096;
for ($i = 0; $i < 256; $i++) {

    foreach ($arr[$i] as $byte)
        $mem[$address++] = $byte;
}

$out = "WIDTH=8;\nDEPTH=8192;\nADDRESS_RADIX=HEX;\nDATA_RADIX=HEX;\nCONTENT BEGIN\n";
for ($i = 0; $i < 8192; $i++) {
    $out .= sprintf("  %04X: %02X;\n", $i, isset($mem[$i]) ? $mem[$i] : 0);
}
$out .= "END;\n";

echo $out;
