#ifndef __USB_DETECTOR_H__
#define __USB_DETECTOR_H__
#include <vector>
#include <string>

extern "C"
{
    #include <sys/types.h>
    #include <pthread.h> 
}

using namespace std;

namespace zc55{
    // #define DEFAULT_RETRIVE_CMD         "ls -l /sys/class/block | grep -e 'usb' -e 'mmc0'"
    #define DEFAULT_INPUT_HOST_ADDR     "fe800000"
    #define DEFAULT_OUTPUT_HOST_ADDR    "fe900000"
    #define DEFAULT_SDCARD_HOST_ADDR    "fe320000"

class UsbDetector{

public:
    UsbDetector(string inputHostAddr, string outputHostAddr, string sdcardHostAddr);
    ~UsbDetector(){};

    static pthread_t threadFunc(void *arg);
    static volatile int execFlag;
    static vector<string> inputUsbDevice;
    static vector<string> outputUsbDevice;
    static vector<string> sdcardDevice;
    
private:
    string cmd;
    string inputHostAddr;   //usb
    string outputHostAddr;  //usb
    string sdcardHostAddr;  //sd card


};

}

#endif