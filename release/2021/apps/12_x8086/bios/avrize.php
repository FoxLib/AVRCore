<?php

$bios = file_get_contents("bios.bin");
$size = strlen($bios);

$map = [];
$out = "const byte BIOS[] PROGMEM = ";

for ($i = 0; $i < $size; $i++) {
    $map[] = sprintf("0x%02x", ord($bios[$i]));
}

$out .= '{'.join(", ", $map).'};';
echo $out;
