CC=g++
SDL=`sdl-config --cflags --libs`
LIBS=-lSDL
WARN=-Wall -Wno-strict-overflow -Wno-unused-result -Wno-unused-variable -Wno-unused-but-set-variable
CFLAGS=-Icore -I. $(SDL) -O3 $(WARN)
OBJ=vm.o io.o main.o

all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o avr
	./avr

%.o: %.cc main.h
	$(CC) $(CFLAGS) $(LIBS) $< -c -o $@

install: avr
	cp avr /usr/local/bin

clean:
	rm -f *.o avr
