CC = gcc

# -D_GNU_SOURCE добавлен для корректной работы POSIX-функций при строгом стандарте
CFLAGS = -c -Wpedantic -Wall -Wextra -g -std=c99 -D_GNU_SOURCE \
-pedantic \
-Wno-error=format-nonliteral -Wformat-nonliteral \
-Wno-error=discarded-qualifiers -Wno-error=incompatible-pointer-types -Wno-clobbered \
-fPIE -fstack-protector-all -ftrapv \
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

# Флаги линковщика вынесены отдельно
LDFLAGS = -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack
INCLUDES = -I./lib

SRCDIR = src
ODIR = obj

# Исходники
SRC = main.c

# Генерация путей для объектных файлов
OBJ = $(addprefix $(ODIR)/, $(notdir $(SRC:.c=.o)))

# Цели
TARGET_DIR = target
TARGET = $(TARGET_DIR)/sniffer.elf

all: $(TARGET)

$(ODIR):
	mkdir -p $@

$(TARGET_DIR):
	mkdir -p $@

$(TARGET): $(OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(ODIR)/%.o: %.c | $(ODIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(ODIR)/%.o: $(SRCDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(ODIR) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all clean