#include <string>
#include <iostream>

extern "C"{
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/ioctl.h>
    #include <string.h>
    #include <signal.h>
    #include <linux/spi/spidev.h>
}

#include "lcd.h"

using namespace std;

namespace zc55{
    Lcd::Lcd(string name):devFd(0)
    {
        this->name = name;
    }

    Lcd::~Lcd(){
        if(this->devFd > 0)
        {
            close(this->devFd);
        }
    }

    int Lcd::init()
    {
        int ret = 0;
        unsigned char initCmd[] = {0xae, 0x20, 0x10, 0xb0, 0xc8, 0x00, 0x10, 0x40,
                                   0x81, 0xff, 0xa1, 0xa6, 0xa8, 0x3f, 0xa4, 0xd3,
                                   0x00, 0xd5, 0xf0, 0xd9, 0x22, 0xda, 0x12, 0xdb,
                                   0x20, 0x8d, 0x14, 0xaf};

        this->devFd = open(this->name.c_str(), O_RDWR);
        if(this->devFd < 0)
        {
            cout << "open lcd :" << this->name << " error" << endl;
            return -1;
        }

        ret =  this->lcd_trans_cmd(initCmd, sizeof(initCmd) / sizeof(unsigned char));
        if(ret < 0)
        {
            cout << "lcd : send init cmd error" << endl;
            return -2;
        }

        return this->clearScreen();
    }

    int Lcd::clearScreen()
    {
        int i;
        unsigned char cmd[3];
        unsigned char data[128] = {0};
        cmd[0] = 0xb0;
        cmd[1] = 0x00;
        cmd[2] = 0x10;
        for(i = 0; i< 8; i++)
        {
            this->lcd_trans_cmd(cmd, 3);
            this->lcd_trans_data(data, 128);
            cmd[0]++;
        }
    }

    int Lcd::lcd_trans_cmd(unsigned char *buf, int n)
    {
        int cs, dc;
        int ret;
        int nwrite;
        cs = CS_ON;
        dc = TRANSFER_CMD;
        ret = ioctl(this->devFd, LCD_SET_CS1, &cs);
        if(ret < 0)
        {
            cout << "lcd : set cs1 on error" << endl;
            return -1;
        }

        ret = ioctl(this->devFd, LCD_SET_DC, &dc);
        if(ret < 0)
        {
            cout << "lcd : set cmd on error" << endl;
            return -1;
        }

        nwrite = write(this->devFd, buf, n);
        if(nwrite < 0)
        {
            cout << "lcd : send cmd error" << endl;
            return -1;
        }

        cs = CS_OFF;
        dc = TRANSFER_DATA;

        ret = ioctl(this->devFd, LCD_SET_CS1, &cs);
        if(ret < 0)
        {
            cout << "lcd : set cs1 off error" << endl;
            return -1;
        }
        ret = ioctl(this->devFd, LCD_SET_DC, &dc);
        if(ret < 0)
        {
            cout << "lcd : set cmd off error" << endl;
            return -1;
        }
        return nwrite;
    }

    int Lcd::lcd_trans_data(unsigned char *buf, int n)
    {
        int cs, dc;
        int ret;
        int nwrite;
        cs = CS_ON;
        ret = ioctl(this->devFd, LCD_SET_CS1, &cs);
        if(ret < 0)
        {
            cout << "lcd : set cs1 on error" << endl;
            return -1;
        }

        nwrite = write(this->devFd, buf, n);
        if(nwrite < 0)
        {
            cout << "lcd : send data error" << endl;
            return -1;
        }
        cs = CS_OFF;
        ret = ioctl(this->devFd, LCD_SET_CS1, &cs);
        if(ret < 0)
        {
            cout << "lcd : set cs1 off error" << endl;
            return -1;
        }
        return nwrite;
    }

    int Lcd::lcd_address(unsigned char col, unsigned char page)
    {
        unsigned char addr[3];
        int ret;
        addr[0] = 0xb0 + page;
        addr[1] = ((col & 0xf0) >> 4) | 0x10;
        addr[2] = (col & 0x0f) | 0x00;
        
        ret =  lcd_trans_cmd(addr, 3);
        if(ret < 0)
        {
            cout << "lcd : set addr error" << endl;
        }
        return ret;
    }

     //x [0,128), y[0, 64)]
     //x + witdth < 128
     //y*8 + height < 64
     //buf size = height/8 * width
     // y % 8 = 0;
    int Lcd::showImage(int x, int y, unsigned int width, unsigned int height, unsigned char *buf) 
    {
        unsigned char page = y / 8;
        unsigned char col = x;
        int npage = (height + 7) / 8;
        int i;
        int ret;

        if((x < 0) || (x > 128))
        {
            cout << "lcd : x value error:" << x << endl;
            return -1;
        }

        if((y < 0) || (y > 64))
        {
            cout << "lcd : y value error: " << y << endl;
            return -1;
        }

        if((x + width) > 128)
        {
            cout << "lcd : width value error:"<<width << endl;
            return -1;
        }

        if((y + height) > 64)
        {
            cout << "lcd : height value error:" <<height<< endl;
            return -1;
        }



        
        for(i = 0; i < npage; i++)
        {
            ret = this->lcd_address(x , page);
            if(ret < 0)
            {
               return -1;
            }
            ret = this->lcd_trans_data(buf, width);
            if(ret < 0)
            {
               return -1;
            }
            buf += width;
            page++;
        }
        
        return 0;
    }

    int Lcd::getCharacterFromRom(unsigned char addr_high, unsigned char addr_mid, unsigned char addr_low,
                            unsigned char *buf, unsigned char len)
    {
        int cs = CS_ON;
        unsigned char addr[4];
        
        addr[0] = 0x03;
        addr[1] = addr_high;
        addr[2] = addr_mid;
        addr[3] = addr_low;


        ioctl(this->devFd, LCD_SET_CS2, &cs);
        write(this->devFd, addr, 4);

        read(this->devFd, buf, len);

        cs = CS_OFF;
        ioctl(this->devFd, LCD_SET_CS2, &cs);

        return 0;
    }

    int Lcd::showStr(int x, int y, unsigned char *text)
    {
        int i = 0;
        unsigned int addr;
        unsigned char addr_high, addr_mid, addr_low;
        unsigned char font_buf[32];
        unsigned char page = (y+7) / 8;
        unsigned char col = x;
        while(text[i] > 0x00)
        {
            if(((text[i] >= 0xb0) && (text[i] <= 0xf7)) && (text[i+1] >= 0xa1))
            {
                addr = (text[i] - 0xb0) *94;
                addr += (text[i + 1] - 0xa1) + 846;
                addr *= 32;
                addr_high = (addr >> 16) & 0xff;
                addr_mid = (addr >> 8) & 0xff;
                addr_low = addr & 0xff;
                this->getCharacterFromRom(addr_high, addr_mid, addr_low, font_buf, 32);
                this->showImage(col, y, 16, 16, font_buf);
                i += 2;
                col += 16;
            }
            else if(((text[i] >= 0xa1) && (text[i] <= 0xa3)) && (text[i+1] >= 0xa1))
            {
                addr = (text[i] - 0xa1) *94;
                addr += (text[i + 1] - 0xa1) + 846;
                addr *= 32;
                addr_high = (addr >> 16) & 0xff;
                addr_mid = (addr >> 8) & 0xff;
                addr_low = addr & 0xff;
                this->getCharacterFromRom(addr_high, addr_mid, addr_low, font_buf, 32);
                this->showImage(col, y, 16, 16, font_buf);
                i += 2;
                col += 16;
            }
            else if((text[i] >= 0x20) && (text[i] < 0x7e))
            {
                addr = (text[i] - 0x20);
                addr *= 16;
                addr += 0x3cf80;
            
                addr_high = (addr >> 16) & 0xff;
                addr_mid = (addr >> 8) & 0xff;
                addr_low = addr & 0xff;
                this->getCharacterFromRom(addr_high, addr_mid, addr_low, font_buf, 16);
                this->showImage(col, y, 8, 16, font_buf);
                i += 1;
                col += 8;
            }
            else
                i++;

        }
    }

}