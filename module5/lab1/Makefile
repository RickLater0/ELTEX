KDIR:=/lib/modules/$(shell uname -r)/build
TDIR:=$(PWD)/target

obj-m += hello.o

MAKEF:=CC=x86_64-linux-gnu-gcc -C $(KDIR) M=$(PWD)
#я добьюсь чистоты в папке
all:
	mkdir -p $(TDIR)
	$(MAKE) $(MAKEF) modules
	-mv -t $(TDIR) .*.o *.o .*.cmd *.mod *.mod.c *.symvers *.order


clean:
	$(MAKE) $(MAKEF) clean
	-rm -rf $(TDIR)

.PHONY: all clean