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
CLIENT_SRC = client.c $(SRCDIR)/common.c $(SRCDIR)/htbl.c
SERVER_SRC = server.c $(SRCDIR)/common.c $(SRCDIR)/htbl.c

# Генерация путей для объектных файлов
CLIENT_OBJ = $(addprefix $(ODIR)/, $(notdir $(CLIENT_SRC:.c=.o)))
SERVER_OBJ = $(addprefix $(ODIR)/, $(notdir $(SERVER_SRC:.c=.o)))

# Цели
TARGET_DIR = target
CLIENT_TARGET = $(TARGET_DIR)/client.elf
SERVER_TARGET = $(TARGET_DIR)/server.elf

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(ODIR):
	mkdir -p $@

$(TARGET_DIR):
	mkdir -p $@

$(CLIENT_TARGET): $(CLIENT_OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(SERVER_TARGET): $(SERVER_OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(ODIR)/%.o: %.c | $(ODIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(ODIR)/%.o: $(SRCDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(ODIR) $(TARGET_DIR)

.PHONY: all clean