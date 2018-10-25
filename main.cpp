#include <iostream> 
#include "led/Led.h"
#include "key/Key.h"

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


using namespace std;
using namespace zc55;
volatile int execFlag = 1;
pthread_t key_thread = 0;

void signal_handler(int sig)
{
    cout << "main thread end" << endl;
    execFlag = 0;
    Key::threadFlag = 0;
}

int  main()
{
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGSTOP, signal_handler);

    Led led(ERR_LED_NAME);
    if(led.ledInit() < 0 )
    {
        return -1;
    }

    Key key;
    key_thread = key.keyInit();

    while(execFlag)
    {
        led.lightOff();
        sleep(5);
        led.lightOn();
        sleep(5);
        led.startBlink(200,200);
        sleep(5);
        led.stopBlink();
    
    }   

    pthread_join(key_thread, NULL);

    return 0;
}

