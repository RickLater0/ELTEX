##Компилик
CC=gcc
##Флаги
CFLAGS=-c -Wpedantic -Wall -Wextra -g -std=c11
TARGET=phonebook
OBJ=obj/main.o obj/Person.o obj/Contacts.o 
all: $(TARGET)

$(TARGET): $(OBJ) libstorage.so
	$(CC) $(OBJ) -o $(TARGET) -L. -lstorage -Wl,--rpath='$$ORIGIN'
#$@ - название цели
#@mkdir скрывает вывод
obj/Person.o: lib/Person.c lib/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) lib/Person.c -o $@

obj/Storage.o: lib/Storage.c lib/Storage.h
	@mkdir -p obj

	$(CC) $(CFLAGS) -fPIC lib/Storage.c -o $@

obj/Contacts.o: Contacts.c Contacts.h lib/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) Contacts.c -o $@

obj/main.o: main.c Contacts.h lib/Person.h
	@mkdir -p obj
	$(CC) $(CFLAGS) main.c -o $@

libstorage.so: obj/Storage.o
	$(CC) -shared $^ -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean