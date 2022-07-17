obj-m := UNO_driver.o

all:
	$ make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$ make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
deploy:
	scp*.ko pi@192.168.137.14:/home/pi/DD/chr_dev
