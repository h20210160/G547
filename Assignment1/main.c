#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/random.h>						// for get_random_bytes() function
#include "chardev.h"

#define DEVICE "Device_One"
//static dev_t dev_num;
//static struct cdev adc_dev;
//static struct class *dev_cls;
static int config[3];
unsigned static short int adc_data,i,x;
unsigned short int adc (void);

//step 4

static int adc_open(struct inode *i,struct file *f)			// open system call
{
	printk(KERN_INFO "\n ADC OPENED\n ");
	return 0;
}

static int adc_close(struct inode *i,struct file *f)			// close system call
{
	
	printk(KERN_INFO "\n ADC CLOSED\n");
	return 0;
}

static ssize_t adc_read(struct file *f, char __user *buf, size_t len, loff_t *off)			// ADC read system call
{
	
	printk(KERN_INFO "READ CALL\n");
	printk(KERN_INFO "\n Channel %d ",config[0]);
	
	if(config[1])	
		printk(KERN_INFO "\n Data is right alligned. ");
	else	
		printk(KERN_INFO "\n Data is left alligned. ");
	
	if(config[2])
	{	
		i=1;
		while(i<10)						// here while(1) could be used to read in continuous mode
		{ 
			adc_data=adc();
			x=copy_to_user(buf,&adc_data,2); 		// to send data from kernel space to userspace
			i+=1;
		}
	}
	else
	{
		adc_data=adc();
		x=copy_to_user(buf,&adc_data,2);
	}
	return 0;
	
}

static ssize_t adc_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{

	printk(KERN_INFO "WRITE\n");
	return 0;
	
}

long adc_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int *temp;
	int c;
	switch(ioctl_num)
	{
		case CHANNEL_SELECT:
			temp=(int *)ioctl_param;
			c=copy_from_user(&config[0],temp,4);				// to get data from userspace to kernel space
			adc_write( f,(char *)ioctl_param,4,0);
			break;
		
		case DATA_ALLIGN:
			temp=(int *)ioctl_param;
			c=copy_from_user(&config[1],temp,4);				// to get data from userspace to kernel space
			adc_write( f,(char *)ioctl_param,4,0);
			break;
		
		case CONVERSION_MODE:
			temp=(int *)ioctl_param;
			c=copy_from_user(&config[2],temp,4);				// to get data from userspace to kernel space
			adc_write( f,(char *)ioctl_param,4,0);
			break;
		
		case ADC_READ:
			adc_read( f, (char *)ioctl_param,2,0);			
			break;
	}
	return 0;
	
}

// file operations structure

struct file_operations fops = {
        .read = adc_read,
        .write = adc_write,
        .open = adc_open,
        .unlocked_ioctl= adc_ioctl,
        .release = adc_close,      /* a.k.a. close */
};

static int __init adc_dev_init(void)
{
	
	//step 1
	
	if(register_chrdev(MAJOR_NUM, DEVICE, &fops)<0)
	{
		return -1;
	}
	printk(KERN_ALERT "ADC DRIVER REGISTERED\n");
	
	
	
	// **************** Following steps not used as device node was created manually***********************
	//step 2
	
	/*if((dev_cls=class_create(THIS_MODULE,"ADC_DEV"))==NULL)
	{
		unregister_chrdev(MAJOR_NUM,DEVICE);
		printk(KERN_INFO "My char driver unregistered\n");
		return -1;
	}
	if(device_create(dev_cls,NULL, dev_num, NULL, DEVICE)==NULL)
	{
		class_destroy(dev_cls);
		unregister_chrdev_region(dev_num,1);
		printk(KERN_INFO "My char driver unregistered\n");
		return -1;
	}	
	
	//step3
	
	cdev_init(&adc_dev, &fops);
	if(cdev_add(&adc_dev, dev_num, 1)==-1)
	{
		device_destroy(dev_cls,dev_num);
		class_destroy(dev_cls);
		unregister_chrdev_region(MAJOR_NUM,1);
		printk(KERN_INFO "My char driver unregistered\n");
		return -1;
	}*/
	return 0;
}

static void __exit adc_dev_exit(void)
{
	/*cdev_del(&ch_dev);
	device_destroy(dev_cls,dev_num);
	class_destroy(dev_cls);*/
	unregister_chrdev(MAJOR_NUM, DEVICE);
	printk(KERN_ALERT "ADC DRIVER UNREGISTERED\n");
}

unsigned short int adc ()
{
	unsigned short int n;
	get_random_bytes(&n,2); 					// generate random number
	n=(n & 0x0FFF );
	if(!config[1])
	{
		printk(KERN_INFO "\n Before left allignment %d ",n);
		n=n*16;
		printk(KERN_INFO "\n After left allignment %d ",n);
	}
	else	printk(KERN_INFO "\n %d ",n);
	return n;
}

module_init(adc_dev_init);
module_exit(adc_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sarath");
MODULE_DESCRIPTION("ADC module");
