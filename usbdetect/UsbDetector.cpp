#include <iostream>

extern "C"
{
    #include <stdio.h> 
    #include <stdlib.h> 
    #include <unistd.h> 
    #include <string.h> 
    #include <sys/types.h> 
    #include <fcntl.h> 
    #include <errno.h> 
    #include <time.h> 
    #include <linux/input.h> 
    #include <sys/select.h>
    #include <time.h>
    #include <pthread.h>
}

#include "UsbDetector.h"
#include "../common/common.h"

namespace zc55{
    volatile int UsbDetector::execFlag = 1;
    vector<string> UsbDetector::inputUsbDevice;
    vector<string> UsbDetector::sdcardDevice;
    vector<string> UsbDetector::outputUsbDevice;

    UsbDetector::UsbDetector(string inputHostAddr, string outputHostAddr, string sdcardHostAddr)
    {
        this->sdcardHostAddr = sdcardHostAddr;
        this->inputHostAddr = inputHostAddr;
        this->outputHostAddr = outputHostAddr;

        UsbDetector::inputUsbDevice.clear();
        UsbDetector::sdcardDevice.clear();
        UsbDetector::outputUsbDevice.clear();

        #if DEBUG_PRINT > 0
           cout << "sdcard host addr: "<< sdcardHostAddr << endl;
           cout << "input host addr: "<< inputHostAddr << endl;
           cout << "output host addr: "<< outputHostAddr << endl;
        #endif

        FILE *fstream = NULL;
        char buf[1024] = {0};
        sprintf(buf, "ls -l /sys/class/block | grep -e '%s' -e '%s' -e '%s'", 
            this->inputHostAddr.c_str(),
            this->outputHostAddr.c_str(),
            this->sdcardHostAddr.c_str()
            );

        #if DEBUG_PRINT > 0
           printf("usb det cmd line :%s\n", buf);
        #endif

        if(NULL == (fstream = popen(buf, "r")))   //查询sd，usb host端接入的设备
        {
            cout<<"usbdetector exec cmd error" << endl;
            return;
        }

        memset(buf, 0, sizeof(buf));
        while(NULL != fgets(buf, sizeof(buf), fstream))
        {
            #if DEBUG_PRINT > 0
            printf("read from ls: %s \n", buf);
            #endif

            string str(buf);
            cout << "str size: "<< str.size() <<endl;
            size_t pos;
            if(str.find(this->inputHostAddr, 0) != string::npos)  //input usb
            {
                if((pos = str.rfind("sd")) != string::npos)
                {
                    cout <<"in usb pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    UsbDetector::inputUsbDevice.push_back(devName);
                }                
            }       

            if(str.find(this->outputHostAddr, 0) != string::npos)  //output usb
            {
                if((pos = str.rfind("sd")) != string::npos)
                {
                    cout <<"in usb pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    UsbDetector::outputUsbDevice.push_back(devName);
                }       
            }         

            if(str.find(this->sdcardHostAddr, 0) != string::npos)  //output usb
            {
                if((pos = str.rfind("mmcblk")) != string::npos)
                {
                    cout <<"in usb pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    UsbDetector::sdcardDevice.push_back(devName);
                }       
            }   

        }

        #if DEBUG_PRINT > 0

        vector<string>::iterator it;
        cout << "usb input device:"<<endl;
        for(it = UsbDetector::inputUsbDevice.begin(); it != UsbDetector::inputUsbDevice.end(); it++)
        {
            cout<<" "<< *it << endl;
        }

        cout << "usb output device:"<<endl;
        for(it = UsbDetector::outputUsbDevice.begin(); it != UsbDetector::outputUsbDevice.end(); it++)
        {
            cout<<" "<< *it << endl;
        }

        cout << "sd card device:"<<endl;
        for(it = UsbDetector::sdcardDevice.begin(); it != UsbDetector::sdcardDevice.end(); it++)
        {
            cout<<" "<< *it << endl;
        }

        #endif

    }
    

    pthread_t UsbDetector::threadFunc(void *arg)
    {

    }

}