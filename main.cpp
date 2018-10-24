#include <iostream> 
#include "led/Led.h"

extern "C"
{
    #include <stdlib.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <string.h>
    #include <fcntl.h>
}

int  main()
{
    zc55::Led led(ERR_LED_NAME);
    
    while(1)
    {
        led.lightOff();
        sleep(5);
        led.lightOn();
        sleep(5);
        led.startBlink(200,200);
        sleep(5);
        led.stopBlink();
    
    }   

    return 0;
}

