# AVRCore
Здесь складываются процессоры на основе AVR технологии

# Составные части

* dbg - Специально разработанный отладчик для AVR
* de0 - Проект для DE0
* mc3 - Проект для Marsohod 2 Cyclone III
* apps - Приложения для запуска
* core - Ядро на verilog
* include - Включаемые модули для apps/

# Схема памяти ATMEGA328

```
     0     1     2      3      4      5      6      7      8     9     A      B      C      D      E      F
20 | ...   ...   ...    PINB   DDB    PORTB  PINC   DDC    PORTC PIND  DDD    PORTD  ...    ...    ...    ...
30 | ...   ...   ...    ...    ...    TIFR0  TIFR1  TIFR2  ...   ...   ...    PCIFR  EIFR   EIMSK  GPIOR0 EECR
40 | EEDR  EEARL EEARH  GTCCR  TCCR0A TCCR0B TCNT0  OCR0A  OCR0B ...   GPIOR1 GPIOR2 SPCR   SPSR   SPDR   ...
50 | ACSR  ...   ...    SMCR   MCUSR  MCUCR  ...    SPMCSR ...   ...   ...    ...    ...    SPL    SPH    SREG
```
