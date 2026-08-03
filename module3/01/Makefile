##Компилятор
CC=gcc
##Флаги
CFLAGS = -c -Wpedantic -Wall -Wextra -g -std=c23 \
-pedantic \
-Wno-error=format-nonliteral -Wformat-nonliteral \
-Wno-error=discarded-qualifiers -Wno-error=incompatible-pointer-types -Wno-clobbered \
-fPIE -fstack-protector-all -ftrapv \
-Wl,-z,relro,-z,now -Wl,-z,noexecstack \
-Wbad-function-cast -Wcast-align -Wconversion \
-Wformat-security -Wformat-truncation \
-Winline -Winvalid-pch -Wlogical-op -Wmissing-declarations \
-Wmissing-prototypes -Wnested-externs -Wold-style-definition \
-Woverlength-strings \
-Wsuggest-attribute=const -Wtrampolines \
-Warray-bounds \
-fno-strict-aliasing \
-fdiagnostics-color=always -Wno-address-of-packed-member \
-Werror -Werror=extra -Werror=write-strings -Werror=cast-qual \
-Werror=float-equal -Werror=format-overflow=2 -Werror=jump-misses-init -Werror=redundant-decls \
-Werror=shadow -Werror=strict-overflow=5 -Werror=switch-default -Werror=undef -Werror=unused \
-Werror=stringop-overflow=4

TARGET=copy.elf
ODIR=obj
OBJ=$(ODIR)/main.o 
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)


$(ODIR)/main.o: main.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean