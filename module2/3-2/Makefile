##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c99 
TARGET=nettest
ODIR=obj
OBJ=$(ODIR)/main.o
all: $(TARGET)
#lm - link math
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)
#$@ - название цели
#@mkdir скрывает вывод

$(ODIR)/main.o: main.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean