##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c99 
TARGET=chmo
ODIR=obj
OBJ=$(ODIR)/chmo.o $(ODIR)/main.o
all: $(TARGET)
#lm - link math
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)
#$@ - название цели
#@mkdir скрывает вывод
$(ODIR)/chmo.o: chmo.c chmo.h
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) chmo.c -o $@

$(ODIR)/main.o: main.c chmo.h
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean