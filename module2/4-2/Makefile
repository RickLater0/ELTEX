##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c11 
TARGET=pqueue
ODIR=obj
OBJ=$(ODIR)/main.o $(ODIR)/queue.o
all: $(TARGET)
#lm - link math
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)
#$@ - название цели
#@mkdir скрывает вывод
$(ODIR)/queue.o: queue.c queue.h
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) queue.c -o $@


$(ODIR)/main.o: main.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean