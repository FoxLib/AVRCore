all:
	# Ассемблирование
	avr-as -mmcu=avr5 main.asm -o main.o
	avr-ld  main.o -o main.elf
	avr-objcopy -j .text -j .data -O binary main.elf main.bin
	avr-objdump -S main.elf > main.lst

	# Генерация тестовых файлов
	php ../../utils/hexer.php main.bin > ../../tb.hex
	php ../../utils/bin2mif.php main.bin > ../../de0/program.mif

	# Очистка
	rm main.elf main.o
	cd ../.. && make

