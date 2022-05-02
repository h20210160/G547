obj-m := driver_bus.o driver_client.o

all:
	$ make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$ make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
deploy:
	scp*.ko pi@192.168.137.14:/home/pi/DD/chr_dev
