<?php

$im = imagecreatefrompng("brick3.png");
$rows = [];
for ($y = 0; $y < 16; $y++) {

    $cols = [];
    for ($x = 0; $x < 16; $x++) {

        $cl = imagecolorat($im, $x, $y);
        $r  = ($cl >> 16) & 255;
        $g  = ($cl >> 8) & 255;
        $b  = ($cl) & 255;
        $cl = ($r&0xE0) | (($g&0xE0)>>3) | ($b>>6);
        $cols[] = sprintf("0x%02X", $cl);
    }

    $rows[] = "    {".join(", ", $cols) . "}";
}
echo "{\n".join(",\n", $rows) . "\n}\n";
