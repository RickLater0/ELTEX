obj-m += hello.o

KDIR:=/lib/modules/$(shell uname -r)/build
TDIR:=$(PWD)/target

all: | $(TDIR)
	make -C $(KDIR) M=$(PWD) modules
	mv *.ko $(TDIR)/

$(TDIR):
	mkdir -p $(TDIR)

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -rf $(TDIR)

.PHONY: all clean