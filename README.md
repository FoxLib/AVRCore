# AVRCore

Здесь складываются процессоры на основе AVR технологии. Версии
складываются в папку release

# Составные части каждого релиза

* `av` - Отладчик для AVR
* `apps` - Приложения для запуска
* `include` - Включаемые модули для apps/
* `machine` - Ядро на Verilog и TestBench
* `machine/de0` - Проект для DE0
* `machine/mc3` - Проект для Marsohod 2 Cyclone III

# Схема памяти ATMEGA328
```
   0    1     2     3     4      5      6     7
20 ...  ...   ...   PINB  DDB    PORTB  PINC  DDC
30 ...  ...   ...   ...   ...    TIFR0  TIFR1 TIFR2
40 EEDR EEARL EEARH GTCCR TCCR0A TCCR0B TCNT0 OCR0A
50 ACSR ...   ...   SMCR  MCUSR  MCUCR  ...   SPMCSR

   8     9    A      B      C    D     E      F
28 PORTC PIND DDD    PORTD  ...  ...   ...    ...
38 ...   ...  ...    PCIFR  EIFR EIMSK GPIOR0 EECR
48 OCR0B ...  GPIOR1 GPIOR2 SPCR SPSR  SPDR   ...
58 ...   ...  ...    RAMPZ  ...  SPL   SPH    SREG
```

# Схема памяти 46К
```
32k ROM Память программ  $0000-$3FFF
10k RAM Общая память     $0000-$37FF
 4k RAM Видеопамять      $F000-$FFFF
```
