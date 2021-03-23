# AVRCore
Здесь складываются процессоры на основе AVR технологии

# Составные части

* `dbg` - Специально разработанный отладчик для AVR
* `de0` - Проект для DE0
* `mc3` - Проект для Marsohod 2 Cyclone III
* `apps` - Приложения для запуска
* `core` - Ядро на verilog
* `include` - Включаемые модули для apps/

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
58 ...   ...  ...    ...    ...  SPL   SPH    SREG
```

# Схема памяти 46К
```
16k ROM Память программ  $0000-$3FFF
14k RAM Общая память     $0000-$37FF
16k RAM Видеопамять      $B800-$F7FF
```

# Кастомные порты
```
R  20 Данные с клавиатуры
R  21 Бит 0: Триггер нажатий на клавиатуру
```

