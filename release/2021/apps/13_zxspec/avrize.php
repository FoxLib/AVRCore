<?php

$bios = file_get_contents("48k.rom");
$size = strlen($bios);

$map = [];
$out = "const byte ROMBIOS[] PROGMEM = \n";

for ($i = 0; $i < $size; $i++) {
    $map[] = sprintf("0x%02x", ord($bios[$i])) . ($i%16 == 15 ? "\n" : "");
}

$out .= '{'.join(", ", $map).'};';
echo $out;
