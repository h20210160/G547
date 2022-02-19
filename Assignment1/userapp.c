
// Device node creation was done manually using mknod command

#include "chardev.h"		// contains major number , device file name, generates magic number

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>              /* open */
#include <unistd.h>             /* exit */
#include <sys/ioctl.h>          /* ioctl */

/*
 * Functions for the ioctl calls
 */

int channel_select(int file_desc, int *channel)			// fuction to select channel
{
	int ret_val;

	ret_val = ioctl(file_desc, CHANNEL_SELECT, channel);		// ioctl userspace call

    	if (ret_val < 0) 
    	{
        	printf("Channel selection failed:%d\n", ret_val);
        	exit(-1);
    	}
	return 0;
}

int data_allignment(int file_desc, int *R_L )				// fuction to select allignment
{
    	int ret_val;

	ret_val = ioctl(file_desc, DATA_ALLIGN, R_L);			// ioctl userspace call

    	if (ret_val < 0) 
    	{
        	printf("Data allignment failed:%d\n", ret_val);
        	exit(-1);
    	}
    	return 0;

}

int conversion_mode_change(int file_desc, int *mode)			// fuction to select continuous or one shot mode
{
    
	int ret_val;
        ret_val = ioctl(file_desc, CONVERSION_MODE, mode);		// ioctl userspace call

        if (ret_val < 0) 
	{
		printf("Conversion mode change failed\n");
        	exit(-1);
        }
        return 0;
}

short int adc_read(int file_desc)					// fuction to read ADC data
{
    
	int ret_val;
	short int data;
        ret_val = ioctl(file_desc, ADC_READ, &data);			// ioctl userspace call

        if (ret_val < 0) 
	{
		printf("Read operation failed\n");
        	exit(-1);
        }
        return data;
}

/*
 * Main - Call the ioctl functions
 */
int main()
{
    	int file_desc,flag;
    	int c_no,aln,m,rem[16],i,n;
    	unsigned short int dat;
    
    	file_desc = open(DEVICE_FILE_NAME, 1);					// opening device file
    
    	if (file_desc < 0) 
    	{
    		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        	exit(-1);
    	}

    	printf("\n Select channel number (0-7) : ");
    	scanf("%d",&c_no);
    	channel_select( file_desc, &c_no);
    	printf("\n Select data allignment\n Enter '1' to right allign, '0' to left allign : ");
    	scanf("%d",&aln);
    	data_allignment( file_desc, &aln );
    	printf("\n Select conversion mode\n Enter '1' for continuous conversion mode, '0' for one shot conversion : ");
    	scanf("%d",&m);
    	conversion_mode_change( file_desc, &m );
    	printf("\n Press '0' to read ADC data : ");
    	scanf("%d",&flag);
    	if(!flag)
    	{
		dat=adc_read(file_desc);
		printf("\n Latest ADC Data (decimal) : %d \n",dat);
		i=0;
		n=dat;
		while(n/2)			// to convert the data to binary value
		{
			rem[i]=n%2;
			n/=2;
			i+=1;
		}
		rem[i]=n%2;
		printf("\n Latest ADC Data (binary) : ");
		
		for(int j=i;j>=0;j--)	printf("%d",rem[j]);			// to print the binary value
		printf("\n");
	}
    	close(file_desc);				// closes the device file
    	return 0;
}
