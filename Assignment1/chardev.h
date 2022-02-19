#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>

/*
 * The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it.
 */
#define MAJOR_NUM 160

/*
 * Set the configuration of the device driver
 */
#define CHANNEL_SELECT _IOW(MAJOR_NUM, 0, int * )

#define DATA_ALLIGN _IOW(MAJOR_NUM, 1, int * )

#define CONVERSION_MODE _IOW(MAJOR_NUM, 2, int * )

/*
 * _IOR means that we're creating an ioctl command
 * number for passing information from a user process
 * to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device
 * number we're using.
 *
 * The second argument is the number of the command
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from
 * the process to the kernel.
 */

/*
 * Get the data of the device driver
 */


#define ADC_READ _IOR(MAJOR_NUM, 3, short int * )


/*
 * The name of the device file
 */
#define DEVICE_FILE_NAME "/dev/adc_dev"

#endif
