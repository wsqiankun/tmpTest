#include <iostream> 

extern "C"
{
    #include <stdlib.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <string.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <pthread.h>
}

#include "led/Led.h"
#include "key/Key.h"
#include "usbdetect/UsbDetector.h"
#include "FPGA/fpga.h"

using namespace std;
using namespace zc55;
volatile int execFlag = 1;
Fpga *fpga;
unsigned char * fpga_buf;

void hexdump(char *buf, int n)
{
    printf("hexdump:\n");
    int i = 0;
    for(i = 0; i < n; i++)
    {
        printf("%02X ", buf[i]);
        if(((i % 16) == 0) && (i > 0))
            printf("\n");
    }
    printf("\n");
}

void signal_handler(int sig)
{
    cout << "main thread end" << endl;
    execFlag = 0;  //main thread stop
    Key::execFlag = 0; //key thread stop
    UsbDetector::execFlag = 0;
}

void genDataOnKey(unsigned char keyCode, KeyStatus pressed)
{
    if(pressed == KEY_RELEASED)
    {
        fpga->genData();
        hexdump((char *)fpga_buf, 256);
    }
}

int  main()
{
    pthread_t key_thread = 0;
    pthread_t usb_thread = 0;

    char seed_str[] = "hello 012345";
    struct fpga_seed seed;

    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSTOP, signal_handler);
    signal(SIGQUIT, signal_handler);

    

    fpga = new Fpga("/dev/fpga-enc");
    fpga_buf = fpga->init();
    seed.seed = (unsigned char *)seed_str;
    seed.seed_len = strlen(seed_str);
    if(fpga->setSeed(&seed) < 0)
    {
        return -1;
    }

    Led led(ERR_LED_NAME);
    if(led.ledInit() < 0 )
    {
        return -1;
    }

    led.startBlink(200,200);
    Key key("gpio-keys");
    key.setKeyCB(genDataOnKey);
    key_thread = key.startKeyMonitor();
    if(key_thread <= 0)
    {
        cout << "key thread start error" << endl;
        return -1;
    }

    
    UsbDetector usb(DEFAULT_INPUT_HOST_ADDR, 
                    DEFAULT_OUTPUT_HOST_ADDR,
                    DEFAULT_SDCARD_HOST_ADDR);
    usb_thread = usb.startMonitor();
    if(usb_thread <= 0)
    {
        cout << "usb thread start error" << endl;
        Key::execFlag = 0;
        return -1;
    }

    
    

    pthread_join(key_thread, NULL);
    pthread_join(usb_thread, NULL);
    
    delete(fpga);
    cout << "bye bye" << endl;
    return 0;
}

