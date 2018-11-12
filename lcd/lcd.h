#ifndef __LCD_H__
#define __LCD_H__

#include <string>
extern "C"
{
    #include <linux/spi/spidev.h>
}

using namespace std;

namespace zc55{
#define CS_ON 0
#define CS_OFF 1
#define TRANSFER_CMD 0
#define TRANSFER_DATA 1


#define LCD_SET_CS1  _IOW(SPI_IOC_MAGIC, 98, __u32)
#define LCD_SET_CS2  _IOW(SPI_IOC_MAGIC, 99, __u32)
#define LCD_SET_DC  _IOW(SPI_IOC_MAGIC, 100, __u32)

class Lcd{
public:
    Lcd(string name);
    ~Lcd();
    int init();
    int clearScreen();
    int showImage(int x, int y, unsigned int width, unsigned int height, unsigned char *buf);
    int showStr(int x, int y, unsigned char *text);
private:
    string name;
    int devFd;
    int lcd_trans_cmd(unsigned char *buf, int n);
    int lcd_trans_data(unsigned char *buf, int n);
    int lcd_address(unsigned char col, unsigned char page);
    int getCharacterFromRom(unsigned char addr_high, unsigned char addr_mid, unsigned char addr_low,
                            unsigned char *buf, unsigned char len);

};

}
#endif