#ifndef __LCD_12864_H__
#define __LCD_12864_H__

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#define CS_ON 0
#define CS_OFF 1
#define TRANSFER_CMD 0
#define TRANSFER_DATA 1


#define LCD_SET_CS1  _IOW(SPI_IOC_MAGIC, 98, __u32)
#define LCD_SET_CS2  _IOW(SPI_IOC_MAGIC, 99, __u32)
#define LCD_SET_DC  _IOW(SPI_IOC_MAGIC, 100, __u32)


#endif

