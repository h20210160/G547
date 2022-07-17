/***************************************************************************//**
*  \file       UNO_driver.c
*
*  \details    Connecting RaspberryPi in Master Mode to Arduino in Slave mode using I2C protocol and GPIO subsystem

*
*  \author     Sarath Chandra
*
*  \Tested with Linux raspberrypi 5.15.32-v8+
*
* *******************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/driver.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#define SLAVE_DEVICE_NAME ("UNO1")

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Sarath");
MODULE_DESCRIPTION("UNO_driver");
MODULE_VERSION("0.1");

struct gpio_chip chip; //this is our GPIO chip
const char *gpionames[14] = {"D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","D10","D11","D12","D13"};



static struct i2c_client *UNO_client=NULL;
struct i2c_adapter *UNO_adapter=NULL;

static int UNO_probe(struct i2c_client *UNO_client,const struct i2c_device_id *UNO_id)
{
	printk(KERN_INFO "Uno probed.\n");
	return 0;
}

static int UNO_remove(struct i2c_client *UNO_client)
{
	printk(KERN_INFO "Uno removed.\n");
	return 0;
}


struct i2c_device_id UNO_id[]= {
	{SLAVE_DEVICE_NAME,0},
	{},
};

MODULE_DEVICE_TABLE(i2c,UNO_id);

static struct i2c_driver UNO_driver= {
	.driver= {
		.name =SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe= UNO_probe,
	.remove= UNO_remove,
	.id_table= UNO_id,
};

static struct i2c_board_info uno_i2c_board_info = {
  I2C_BOARD_INFO(SLAVE_DEVICE_NAME, 0x09)
};

static void _gpioa_set(struct gpio_chip *chip , unsigned pin, int value)
{
	
	unsigned char p= pin*10 + value;
	
	gpio_set_value(pin,value);
	
	i2c_master_send(UNO_client, &p, 1); 
	printk(KERN_INFO "Pin %d set to %d \n", pin,value);
}

static int _gpioa_request(struct gpio_chip *chip, unsigned offset)
{
	printk(KERN_INFO "REQUESTED %d \n", offset);
	return 0;
}

static int _gpioa_get(struct gpio_chip *chip, unsigned pin)
{
	// get value through I2C from arduino
	unsigned char buf;
	unsigned char p= pin*10 + 4;
	i2c_master_send(UNO_client, &p, 1);
	i2c_master_recv(UNO_client, &buf, 1); 
	printk(KERN_INFO "Input Pin %d changed to %d ", pin,(int)buf);
	return 0;
}

static int _direction_output(struct gpio_chip *chip ,unsigned pin,int value)
{
        gpio_direction_output(pin,1);
        unsigned char p= pin*10 + 2;
		i2c_master_send(UNO_client, &p, 1);
        printk(KERN_ALERT "set offset %d as output\n", pin);
        return 0;
}

static int _direction_input(struct gpio_chip *chip ,unsigned pin)
{
        gpio_direction_input(pin);
        unsigned char p= pin*10 + 3;
		i2c_master_send(UNO_client, &p, 1);
        printk(KERN_ALERT "set offset %d as input", pin);
        return 0;
}

static int device_init(void)
{
	UNO_adapter=i2c_get_adapter(1);
	printk(KERN_INFO "get adapter\n");
	if(UNO_adapter!=NULL)
	{
		printk(KERN_INFO "Adapter is not NULL\n");
		UNO_client =i2c_new_client_device(UNO_adapter,&uno_i2c_board_info);
		printk(KERN_INFO "new client dev\n");
		if(UNO_client!=NULL)
			{
				printk(KERN_INFO "Client is not NULL\n");
				i2c_add_driver(&UNO_driver);
				printk(KERN_INFO "Driver added\n");
			}
			 i2c_put_adapter(UNO_adapter);
		//i2c_register_driver(THIS_MODULE, &UNO_driver);
		printk(KERN_INFO " i2c client added.\n");
	}
	chip.label = "gpio_i2c";
	//chip.dev = NULL; // optional device providing the GPIOs
	chip.owner = THIS_MODULE; // helps prevent removal of modules exporting active GPIOs, so this is required for proper cleanup
	chip.base = 200; 
	chip.ngpio = 14; 
	chip.can_sleep = false;
	chip.set = _gpioa_set;
	chip.get = _gpioa_get;
	chip.names = gpionames;
	chip.request = _gpioa_request; 	 	 
	chip.direction_input = _direction_input;
	chip.direction_output = _direction_output;


	if (gpiochip_add(&chip) < 0)
	{
	   printk(KERN_ALERT "Failed to add gpio chip. \n");
	   return -ENODEV;
	}
	//int i;
	/*for(i=200;i<214;i++)
	{
		gpio_request(i, NULL);
  		gpio_export(i,1);
  	}*/
	printk(KERN_INFO "GPIO Inserted.\n");
	return 0;
}

static void device_exit(void)
{
   
	
	i2c_unregister_device(UNO_client);
	printk(KERN_ALERT "i2c unreg dev.\n");
	i2c_del_driver(&UNO_driver);
	printk(KERN_ALERT "i2c del ddriver.\n");
	gpiochip_remove(&chip);
	printk(KERN_ALERT "GPIO chip removed.\n");

   
}

module_init(device_init);
module_exit(device_exit);
