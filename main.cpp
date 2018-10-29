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

using namespace std;
using namespace zc55;
volatile int execFlag = 1;


void signal_handler(int sig)
{
    cout << "main thread end" << endl;
    execFlag = 0;  //main thread stop
    Key::execFlag = 0; //key thread stop
    UsbDetector::execFlag = 0;
}

int  main()
{
    pthread_t key_thread = 0;
    pthread_t usb_thread = 0;

    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSTOP, signal_handler);
    signal(SIGQUIT, signal_handler);

    Led led(ERR_LED_NAME);
    if(led.ledInit() < 0 )
    {
        return -1;
    }

    led.startBlink(200,200);
    Key key("gpio-keys");
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
    
    cout << "bye bye" << endl;
    return 0;
}

