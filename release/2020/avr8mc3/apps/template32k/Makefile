# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
DEVICE  = atmega32u2
CLOCK   = 25000000
LIBRARY = -I../../include
AV		= ../../av/avr
WARNING = -Wall -Wno-unused-but-set-variable
STACK   = 0x801fff
HEAPEND = 0x801fff
COMPILE = avr-gcc $(LIBRARY) $(WARNING) -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -fdata-sections -Wl,--gc-sections,--defsym=__stack=$(STACK),--defsym=__heap_end=$(HEAPEND)

all:
	$(COMPILE) main.cc -o main.elf
	avr-objcopy -j .text -j .data -O binary main.elf main.bin
	avr-objdump -S main.elf > main.lst
	avr-size -C --mcu=$(DEVICE) main.elf > mem.map
	rm main.elf
	$(AV) main.bin

clean:
	rm -f *.o rm *.elf *.lst *.bin *.map
