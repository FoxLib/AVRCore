<?php

/**
 * Преобразует бинарные данные в MEMFLASH.MIF
 */

$file = isset($argv[1]) ? $argv[1] : stdin;
$file = file_get_contents($file);
$size = (int)(strlen($file) >> 1);

$out = "WIDTH=16;\nDEPTH=32768;\nADDRESS_RADIX=HEX;\nDATA_RADIX=HEX;\nCONTENT BEGIN\n";
for ($i = 0; $i < $size; $i++) {

    $val = ord($file[2*$i]) + 256*ord($file[2*$i + 1]);
    $out .= sprintf("  %04X: %04X;\n", $i, $val);
}
$out .= sprintf("  [%04X..7FFF]: 0000;\n", $size);
$out .= "END;\n";

echo $out;
