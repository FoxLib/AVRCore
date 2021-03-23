# WARN=-Wall -Wno-strict-overflow -Wno-unused-result -Wno-unused-variable -Wno-unused-but-set-variable

# Опции компилятора
CC=`sdl-config --cflags --libs` -O3

# Файлы для сборки
TARGET=io.o app.o avrdbg.o

# Исполняемый файл
all: $(TARGET)
	g++ $(CC) $(TARGET) -lSDL2 -o avrdbg
	./avrdbg

# Сборка отдельных объектных файлов
%.o: %.cc avrdbg.h
	g++ $(CC) $< -c -o $@

clean:
	rm -f *.o avrdbg
