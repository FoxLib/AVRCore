<?php

$bios = file_get_contents("bios.bin");
$size = strlen($bios);

$map = [];
$out = "const byte BIOS[] PROGMEM = \n";

for ($i = 0; $i < $size; $i++) {
    $map[] = sprintf("0x%02x", ord($bios[$i])) . ($i%16 == 15 ? "\n" : "");
}

$out .= '{'.join(", ", $map).'};';
echo $out;
