DEVICE  = attiny85
CLOCK   = 25000000
LIBRARY = -I../../include
AV		= ../../av/avr
WARNING = -Wall -Wno-unused-but-set-variable
STACK   = 0x80ffff
HEAPEND = 0x807fff
COMPILE = avr-gcc $(LIBRARY) $(WARNING) -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -fdata-sections -Wl,--gc-sections,--defsym=__stack=$(STACK),--defsym=__heap_end=$(HEAPEND)

all:
	$(COMPILE) main.cc -o main.elf
	avr-objcopy -j .text -j .data -O binary main.elf main.bin
	avr-objdump -S main.elf > main.lst
	avr-size -C --mcu=$(DEVICE) main.elf > mem.map
	rm main.elf
	$(AV) main.bin

build:
#	php ../../mc3/hexer.php main.bin > ../../de0/avr.hex
#	php ../../mc3/mifer.php main.bin > ../../de0/flash.mif
#	cd ../../mc3 && make
#	avr main.bin

clean:
	rm -f *.o rm *.elf *.lst *.bin *.map
