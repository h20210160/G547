/* Disk on File Driver */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>


#define DOF_FIRST_MINOR 0
#define DOF_MINOR_CNT 2
#define DOF_SECTOR_SIZE 512
#define DOF_DEVICE_SIZE 1024

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))
#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 // sizeof(PartEntry)
#define PARTITION_TABLE_SIZE 64 // sizeof(PartTable)
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55



typedef struct
{
	unsigned char boot_type; // 0x00 - Inactive; 0x80 - Active (Bootable)
	unsigned char start_head;
	unsigned char start_sec:6;
	unsigned char start_cyl_hi:2;
	unsigned char start_cyl;
	unsigned char part_type;
	unsigned char end_head;
	unsigned char end_sec:6;
	unsigned char end_cyl_hi:2;
	unsigned char end_cyl;
	unsigned int abs_start_sec;
	unsigned int sec_in_part;
} PartEntry;

typedef PartEntry PartTable[4];

static PartTable def_part_table =
{
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x2,
		start_cyl: 0x00,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x09,
		abs_start_sec: 0x00000001,
		sec_in_part: 0x0000013F
	},
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x1,
		start_cyl: 0x14,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x1F,
		abs_start_sec: 0x00000280,
		sec_in_part: 0x00000180
	},
};

static u_int dof_major = 0;

static u8 *dev_data;

/* 
 * The internal structure representation of our Device
 */
static struct dof_device
{
	/* Size is the size of the device (in sectors) */
	unsigned int size;
	/* For exclusive access to our request queue */
	spinlock_t lock;
	/* Our request queue */
	struct request_queue *dof_queue;
	/* This is kernel's representation of an individual disk device */
	struct gendisk *dof_disk;
} dof_dev;

static void copy_mbr(u8 *disk)
{
	
	struct file *fp;
	loff_t pos1=446,pos2=510;
	memset(disk, 0x0, MBR_SIZE);
	char data='A';
	*(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
	memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;
	fp=filp_open("/etc/sample.txt",O_RDWR|O_CREAT,0666);
	kernel_write(fp,&data,MBR_SIGNATURE,&pos2);
	kernel_write(fp,&def_part_table,PARTITION_TABLE_SIZE,&pos1);
	filp_close(fp,NULL);
	
	
}


void dof_write(sector_t sector_offset, u8 *buf, unsigned int sectors)
{
	struct file *fp;
	loff_t pos;
	pos=(loff_t)sector_offset*DOF_SECTOR_SIZE;
	fp=filp_open("/etc/sample.txt",O_WRONLY,0666);
	kernel_write(fp,buf,sectors*DOF_SECTOR_SIZE,&pos);
	filp_close(fp,NULL);
}

void dof_read(sector_t sector_offset, u8 *buf, unsigned int sectors)
{
	struct file *fp;
	loff_t pos;
	pos=(loff_t)sector_offset*DOF_SECTOR_SIZE;
	fp=filp_open("/etc/sample.txt",O_RDONLY,0666);
	kernel_write(fp,buf,sectors*DOF_SECTOR_SIZE,&pos);
	filp_close(fp,NULL);
}






static int dof_open(struct block_device *bdev, fmode_t mode)
{
	unsigned unit = iminor(bdev->bd_inode);

	printk(KERN_INFO "DOF: Device is opened\n");
	printk(KERN_INFO "DOF: Inode number is %d\n", unit);

	if (unit > DOF_MINOR_CNT)
		return -ENODEV;
	return 0;
}

void dof_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "DOF: Device is closed\n");
	
}

/* 
 * Actual Data transfer
 */
static int dof_transfer(struct request *req)
{
	//struct dof_device *dev = (struct dof_device *)(req->rq_disk->private_data);

	int dir = rq_data_dir(req);
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req);

	struct bio_vec bv;
	struct req_iterator iter;

	sector_t sector_offset;
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	printk(KERN_DEBUG "rb: Dir:%d; Sec:%lld; Cnt:%d\n", dir, start_sector, sector_cnt);

	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(bv.bv_page) + bv.bv_offset;
		if (bv.bv_len % DOF_SECTOR_SIZE != 0)
		{
			printk(KERN_ERR "DOF: Should never happen: "
				"bio size (%d) is not a multiple of DOF_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				bv.bv_len, DOF_SECTOR_SIZE);
			ret = -EIO;
		}
		sectors = bv.bv_len / DOF_SECTOR_SIZE;
		printk(KERN_DEBUG "DOF: Sector Offset: %lld; Buffer: %p; Length: %d sectors\n",
			sector_offset, buffer, sectors);
		if (dir == WRITE) /* Write to the device */
		{
			dof_write(start_sector + sector_offset, buffer, sectors);
		}
		else /* Read from the device */
		{
			dof_read(start_sector + sector_offset, buffer, sectors);
		}		
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "DOF: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}
	
/*
 * Represents a block I/O request for us to execute
 */
static void dof_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	/* Gets the current request from the dispatch queue */
	while ((req = blk_fetch_request(q)) != NULL)
	{

		ret = dof_transfer(req);
		__blk_end_request_all(req, ret);
		
	}
}

/* 
 * These are the file operations that performed on the ram block device
 */
static struct block_device_operations dof_fops =
{
	.owner = THIS_MODULE,
	.open = dof_open,
	.release = dof_close,
};
	
/* 
 * This is the registration and initialization section of the ram block device
 * driver
 */
static int __init dof_init(void)
{
	int ret;

	/* Set up our DOF Device 
	if ((ret = dof_device_init()) < 0)
	{
		return ret;
	}
	dof_dev.size = ret;
	*/
	/* Get Registered */
	dof_major = register_blkdev(dof_major, "DOF");
	if (dof_major <= 0)
	{
		printk(KERN_ERR "DOF: Unable to get Major Number\n");
		//ramdevice_cleanup();
		return -EBUSY;
	}

	/* Get a request queue (here queue is created) */
	spin_lock_init(&dof_dev.lock);
	dof_dev.dof_queue = blk_init_queue(dof_request, &dof_dev.lock);
	if (dof_dev.dof_queue == NULL)
	{
		printk(KERN_ERR "dof: blk_init_queue failure\n");
		unregister_blkdev(dof_major, "DOF");
		//ramdevice_cleanup();
		return -ENOMEM;
	}
	
	/*
	 * Add the gendisk structure
	 * By using this memory allocation is involved, 
	 * the minor number we need to pass bcz the device 
	 * will support this much partitions 
	 */
	dof_dev.dof_disk = alloc_disk(DOF_MINOR_CNT);
	if (!dof_dev.dof_disk)
	{
		printk(KERN_ERR "DOF: alloc_disk failure\n");
		blk_cleanup_queue(dof_dev.dof_queue);
		unregister_blkdev(dof_major, "DOF");
		//ramdevice_cleanup();
		return -ENOMEM;
	}

 	/* Setting the major number */
	dof_dev.dof_disk->major = dof_major;
  	/* Setting the first mior number */
	dof_dev.dof_disk->first_minor = DOF_FIRST_MINOR;
 	/* Initializing the device operations */
	dof_dev.dof_disk->fops = &dof_fops;
 	/* Driver-specific own internal data */
	dof_dev.dof_disk->private_data = &dof_dev;
	dof_dev.dof_disk->queue = dof_dev.dof_queue;
	/*
	 * You do not want partition information to show up in 
	 * cat /proc/partitions set this flags
	 */
	//dof_dev.dof_disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
	sprintf(dof_dev.dof_disk->disk_name, "DOF");
	/* Setting the capacity of the device in its gendisk structure */
	set_capacity(dof_dev.dof_disk, dof_dev.size);

	/* Adding the disk to the system */
	add_disk(dof_dev.dof_disk);
	/* Now the disk is "live" */
	printk(KERN_INFO "DOF: Disk on File Block driver initialised (%d sectors; %d bytes)\n",
		dof_dev.size, dof_dev.size * DOF_SECTOR_SIZE);

	return 0;
}
/*
 * This is the unregistration and uninitialization section of the ram block
 * device driver
 */
static void __exit dof_cleanup(void)
{
	del_gendisk(dof_dev.dof_disk);
	put_disk(dof_dev.dof_disk);
	blk_cleanup_queue(dof_dev.dof_queue);
	unregister_blkdev(dof_major, "DOF");
	//ramdevice_cleanup();
}

module_init(dof_init);
module_exit(dof_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email@sarika-pugs.com>");
MODULE_DESCRIPTION("Ram Block Driver");
MODULE_ALIAS_BLOCKDEV_MAJOR(rb_major);
