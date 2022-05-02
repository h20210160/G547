/***************************************************************************//**
*  \file       driver_client.c
*
*  \details    Connecting RaspberryPi in Master Mode to Arduino in Slave mode using I2C protocol and GPIO subsystem

*
*  \author     Sarath, Surbhika, Anubhav
*
*  \Tested with Linux raspberrypi 5.15.32-v8+
*
* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/device.h>
 
#define I2C_BUS_AVAILABLE   (          5 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   ( "ETX_UNO" )              // Device and Driver Name
#define UNO_SLAVE_ADDR  (       0x08 )              // UNO Slave Address
 
static struct i2c_adapter *etx_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *etx_i2c_client_UNO = NULL;  // I2C Cient Structure (In our case it is UNO)
struct gpio_chip chip; //this is our GPIO chip
const char *gpionames[] = {"D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","D10","D11","D12","D13"};


static void UNO_Write(unsigned char );

void send_pin(unsigned offset,int val)
{
    unsigned char data;
    if(offset==12&&val==1)
    data=(unsigned char) offset*10;
    if(offset==12&&val==0)
    {
      data=(unsigned char) offset*10 + 1;
      printk(KERN_INFO " \n off should happen %c\n",data);
      UNO_Write(data);
    }
    UNO_Write(data);
}
static void _gpioa_set(struct gpio_chip *chip, unsigned offset, int value)
{
	printk(KERN_INFO "SET %d to %d \n", offset,value);
  send_pin(offset,value);
}

static int _gpioa_request(struct gpio_chip *chip, unsigned offset)
{
	printk(KERN_INFO "REQUESTED %d \n", offset);
	return 0; 
}

static int _gpioa_get(struct gpio_chip *chip, unsigned offset)
{
   
   printk(KERN_INFO "GPIO GET INFO: %d", offset);
   return 0;
}

static int _direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
        printk(KERN_ALERT "set offset %d output\n", (int)offset);
  	return 0;
}

static int _direction_input(struct gpio_chip *chip, unsigned offset)
{
        printk(KERN_ALERT "set offset %d input", (int)offset);
        return 0;
}
/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  int ret = i2c_master_send(etx_i2c_client_UNO, buf, len);
  printk(KERN_INFO "Inside I2C write function, ret = %d",ret);
  return ret;
}
 
/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  int ret = i2c_master_recv(etx_i2c_client_UNO, out_buf, len);
  printk(KERN_INFO "Inside I2C read function, ret = %d",ret);
  return ret;
}
 

static void UNO_Write( unsigned char data)
{
   int ret;
  
  ret = I2C_Write(&data, 1);
}
 
 
/*
** This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int etx_UNO_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
  
  pr_info("UNO Probed!!!\n");
  
  return 0;
}
 
/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static int etx_UNO_remove(struct i2c_client *client)
{   

  
  pr_info("UNO Removed!!!\n");
  return 0;
}
 
/*
** Structure that has slave device id
*/
static const struct i2c_device_id etx_UNO_id[] = {
  { SLAVE_DEVICE_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, etx_UNO_id);
 
/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver etx_UNO_driver = {
  .driver = {
      .name   = SLAVE_DEVICE_NAME,
      .owner  = THIS_MODULE,
  },
  .probe          = etx_UNO_probe,
  .remove         = etx_UNO_remove,
  .id_table       = etx_UNO_id,
};
 
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info uno_i2c_board_info = {
  I2C_BOARD_INFO(SLAVE_DEVICE_NAME, UNO_SLAVE_ADDR)
};
 
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
  int ret = -1;
  etx_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
  
  if( etx_i2c_adapter != NULL )
  {
      etx_i2c_client_UNO = i2c_new_client_device(etx_i2c_adapter, &uno_i2c_board_info);
      
      if( etx_i2c_client_UNO != NULL )
      {
          i2c_add_driver(&etx_UNO_driver);
          ret = 0;
      }
      
      i2c_put_adapter(etx_i2c_adapter);
  }
  pr_info("Driver Added!!!\n");
  chip.label = "i2c-gpio";
   chip.gpiodev = NULL; // optional device providing the GPIOs
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
	   printk(KERN_ALERT "Failed to add gpio chip");
	   return -ENODEV;
   }
   
  	gpio_request(0, NULL);
  	gpio_request(1, NULL);
   	gpio_export(0,1);
   	gpio_export(1,1);
   	gpio_export(202,1);
   	gpio_export(203,1);
   	gpio_export(204,1);
   	gpio_export(205,1);
   	gpio_export(206,1);
   	gpio_export(207,1);
   	gpio_export(208,1);
   	gpio_export(209,1);
   	gpio_export(210,1);
   printk(KERN_ALERT "Chip Inserted");
  
  return ret;
}
 
/*
** Module Exit function
*/
static void __exit etx_driver_exit(void)
{
	 gpiochip_remove(&chip);
   printk(KERN_ALERT "chip removed");
  i2c_unregister_device(etx_i2c_client_UNO);
  i2c_del_driver(&etx_UNO_driver);
  pr_info("Driver Removed!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarath, Surbhika, Anubhav");
MODULE_DESCRIPTION("Connecting RaspberryPi in Master Mode to Arduino in Slave mode using I2C protocol and GPIO subsystem");
MODULE_VERSION("1.1");

