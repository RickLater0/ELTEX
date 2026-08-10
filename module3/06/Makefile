CC=gcc
CFLAGS = -c -Wpedantic -Wall -Wextra -g -std=c99 \
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

LDFLAGS = -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack

TARGET = groupchat.elf
ODIR = obj

SRC = main.c 
OBJ = $(addprefix $(ODIR)/, $(notdir $(SRC:.c=.o)))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(ODIR):
	mkdir -p $@

$(ODIR)/main.o: main.c | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(ODIR) $(TARGET)

.PHONY: all clean