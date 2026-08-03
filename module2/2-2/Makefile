##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c99 
TARGET=calcu-litr
LTARGET=solib
ODIR=obj
SRC=src
LIB=lib
OBJ=obj
LIBS=$(SRC)/div.c $(SRC)/mod.c $(SRC)/mul.c $(SRC)/pow.c $(SRC)/sub.c $(SRC)/sum.c

OBJ=$(ODIR)/main.o
all: $(TARGET)
#lm - link math
calcu-litr: $(OBJ) $(LTARGET)
	$(CC) $(OBJ) -o $(TARGET) -lm -ldl

$(LTARGET): $(LIB)/libdiv.so $(LIB)/libmod.so $(LIB)/libmul.so $(LIB)/libpow.so $(LIB)/libsub.so $(LIB)/libsum.so
	
$(LIB)/libdiv.so: $(SRC)/div.c
	@mkdir -p $(LIB)
	gcc -shared -fPIC $^ -o $@

$(LIB)/libmod.so: $(SRC)/mod.c
	@mkdir -p $(LIB)
	gcc -shared -fPIC $^ -o $@ -lm

$(LIB)/libmul.so: $(SRC)/mul.c
	gcc -shared -fPIC $^ -o $@

$(LIB)/libpow.so: $(SRC)/pow.c
	gcc -shared -fPIC $^ -o $@ -lm

$(LIB)/libsub.so: $(SRC)/sub.c
	gcc -shared -fPIC $^ -o $@

$(LIB)/libsum.so: $(SRC)/sum.c
	gcc -shared -fPIC $^ -o $@

#$@ - название цели
#@mkdir скрывает вывод

$(ODIR)/main.o: main.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean