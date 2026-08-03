##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c99
TARGET=phonebook
ODIR=obj
SRCDIR=lib
OBJ=obj/main.o obj/Person.o obj/Contacts.o
all: $(TARGET)

phonebook: $(OBJ) libstorage.a
	$(CC) $(OBJ) -o $(TARGET) -L. -lstorage
#$@ - название цели
#@mkdir скрывает вывод
libstorage.a: $(ODIR)/Storage.o
	ar rc $@ $^

obj/Person.o: $(SRCDIR)/Person.c $(SRCDIR)/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(SRCDIR)/Person.c -o $@

obj/Storage.o: $(SRCDIR)/Storage.c $(SRCDIR)/Storage.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(SRCDIR)/Storage.c -o $@

obj/Contacts.o: Contacts.c Contacts.h $(SRCDIR)/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) Contacts.c -o $@

obj/main.o: main.c Contacts.h $(SRCDIR)/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean