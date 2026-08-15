CC = gcc

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

# Флаги линковщика
LDFLAGS = -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack
INCLUDES = -I./lib

SRCDIR = src
ODIR = obj

# Исходники
TAXI_SRC = $(SRCDIR)/taxi.c $(SRCDIR)/tcommon.c $(SRCDIR)/htbl.c
DRIVER_SRC = $(SRCDIR)/driver.c $(SRCDIR)/tcommon.c

TAXI_OBJ = $(addprefix $(ODIR)/, $(notdir $(TAXI_SRC:.c=.o)))
DRIVER_OBJ = $(addprefix $(ODIR)/, $(notdir $(DRIVER_SRC:.c=.o)))

# Цели
TARGET_DIR = target
TAXI_TARGET = $(TARGET_DIR)/taxi.elf
DRIVER_TARGET = $(TARGET_DIR)/driver.elf

all: $(TAXI_TARGET) $(DRIVER_TARGET)

$(ODIR):
	mkdir -p $@

$(TARGET_DIR):
	mkdir -p $@

$(TAXI_TARGET): $(TAXI_OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(DRIVER_TARGET): $(DRIVER_OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(ODIR)/%.o: $(SRCDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(ODIR) $(TARGET)

.PHONY: all clean