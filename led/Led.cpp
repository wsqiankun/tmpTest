#include <iostream>
#include "../common.h"

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

#include <Led.h>

namespace zc55{

    Led::Led(string ledName)
    {
        ledFullName = LEDPATH + ledName + '/';
        ledBtightnessFile = ledFullName + "brightness";
        ledTriggerFile = ledFullName + "trigger";
        ledDelayOnFile = ledFullName + "delay_on";
        ledDelayOffFile = ledFullName + "delay_off";

    #if DEBUG_PRINT > 0
        cout << "led full name :" <<ledFullName << endl;
        cout << "led full name :" <<ledFullName << endl;
        cout << "led full name :" <<ledFullName << endl;
        cout << "led full name :" <<ledFullName << endl;
        cout << "led full name :" <<ledFullName << endl;
    #endif
    }


    int Led::lightOn()
    {
        int n;
        int fd = open(ledBtightnessFile.c_str(), O_WRONLY);
        ledBtightnessFile.data();
        if(fd < 0)
        {
            cout << "open brightness file error" << endl;
            return -1;
        }

        n = write(fd, "255", 3);

        close(fd);
        if(n <= 0)
            return -2;
        
        return 0;
    }

    int Led::lightOff()
    {
        int n;
        int fd = open(ledBtightnessFile.c_str(), O_WRONLY);
        ledBtightnessFile.data();
        if(fd < 0)
        {
            cout << "open brightness file error" << endl;
            return -1;
        }

        n = write(fd, "0", 1);

        close(fd);
        if(n <= 0)
            return -2;
        
        return 0;
    }

    int Led::startBlink(int hz)
    {

    }

    int Led::startBlink(int delayOn, int delayOff)
    {

    }

    int Led::stopBlink()
    {

    }

}