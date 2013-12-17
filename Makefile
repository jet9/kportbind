obj-m += kportbind.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -O2 -o kporttest kporttest.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf kporttest
