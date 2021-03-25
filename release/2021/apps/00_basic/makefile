DEVICE  = atmega103
OPTIONS = -Os -DF_CPU=25000000 -mmcu=$(DEVICE)
INCLUDE = -I../../include
WARNING = -Wall -Wno-unused-but-set-variable
LINKER  = -fdata-sections -Wl,--gc-sections,--defsym=__stack=0x80efff,--defsym=__heap_end=0x80efff
COMPILE = avr-gcc $(INCLUDE) $(WARNING) $(OPTIONS) $(LINKER)
AVR     = ../../av/avr

all:
	$(COMPILE) main.cc -o main.elf
	avr-objcopy -j .text -j .data -O binary main.elf main.bin
	avr-objdump -S main.elf > main.lst
	avr-size -C --mcu=$(DEVICE) main.elf > mem.map
	rm main.elf
	#php ../../de0/hexer.php main.bin > ../../de0/avr.hex
	#php ../../de0/mifer.php main.bin > ../../de0/flash.mif
	#cd ../../de0 && make
	$(AVR) main.bin

clean:
	rm -f *.o rm *.elf *.lst *.bin *.map
