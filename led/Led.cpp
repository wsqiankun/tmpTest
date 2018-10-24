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
    #include <dirent.h>
}

#include "Led.h"

namespace zc55{

    Led::Led(string ledName)
    {
        this->ledName = ledName;
        this->ledFullName = this->LEDPATH + ledName + '/';
        this->ledBtightnessFile = this->ledFullName + "brightness";
        this->ledTriggerFile = this->ledFullName + "trigger";
        this->ledDelayOnFile = this->ledFullName + "delay_on";
        this->ledDelayOffFile = this->ledFullName + "delay_off";

    #if DEBUG_PRINT > 0
        cout << "led full name :" <<this->ledFullName << endl;
        cout << "led brightness name :" <<this->ledBtightnessFile << endl;
        cout << "led trigger name :" <<this->ledTriggerFile << endl;
        cout << "led delayOn name :" <<this->ledDelayOnFile << endl;
        cout << "led delayOff name :" <<this->ledDelayOffFile << endl;
    #endif
    }

    int Led::ledInit()
    {
        if(opendir(this->ledFullName.c_str()) == NULL)
        {
            cout << "led "<< this->ledName << "doesn't exist" << endl;
            return -1;
        }
            
        return 0;
    }

    int Led::lightOn()
    {
        int n;
        int fd = open(this->ledBtightnessFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open brightness file error" << endl;
            return -1;
        }

        n = write(fd, "255", 3);

        close(fd);
        if(n != 3)
        {
            cout << "write 255 to brightness err" << endl;
            return -2;
        }
     
        return 0;
    }

    int Led::lightOff()
    {
        int n;
        int fd = open(this->ledBtightnessFile.c_str(), O_WRONLY);

        if(fd < 0)
        {
            cout << "open brightness file error" << endl;
            return -1;
        }

        n = write(fd, "0", 1);

        close(fd);
        if(n != 1)
        {
            cout << "write 0 to brightness err" << endl;
            return -2;
        }
        
        return 0;
    }

    int Led::startBlink(int hz)
    {
        int fd;
        int n;
        int delayTime;
        char delayBuf[16];

        if(hz <= 0)
        {
            cout << "illegal hz value" << endl;
            return -1;
        }

        delayTime = 1000 / (2*hz);

        if(access(this->ledDelayOnFile.c_str(), F_OK) != 0)
        {
            fd = open(this->ledTriggerFile.c_str(), O_WRONLY);
            if(fd < 0)
            {
                cout << "open trigger file error" << endl;
                return -2;
            }
            n = write(fd, "timer", 5);
            close(fd);
            if(n != 5)
            {
                cout << "write timer to trigger err" << endl;
                return -3;
            }
        }

        fd = open(this->ledDelayOnFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open delayon file error" << endl;
            return -4;
        }
        sprintf(delayBuf, "%d", delayTime);
        n = write(fd, delayBuf, strlen(delayBuf));
        close(fd);

        fd = open(this->ledDelayOffFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open delayOff file error" << endl;
            return -5;
        }
        n = write(fd, delayBuf, strlen(delayBuf));
        close(fd);
        return 0;     

    }

    int Led::startBlink(int delayOn, int delayOff)
    {
        int fd;
        int n;
        char delayBuf[10];

        if(access(this->ledDelayOnFile.c_str(), F_OK) != 0)
        {
            fd = open(this->ledTriggerFile.c_str(), O_WRONLY);
            if(fd < 0)
            {
                cout << "open trigger file error" << endl;
                return -2;
            }
            n = write(fd, "timer", 5);
            close(fd);
            if(n != 5)
            {
                cout << "write timer to trigger err" << endl;
                return -3;
            }
        }

        fd = open(this->ledDelayOnFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open delayon file error" << endl;
            return -4;
        }
        sprintf(delayBuf, "%d", delayOn);
        n = write(fd, delayBuf, strlen(delayBuf));
        close(fd);

        fd = open(this->ledDelayOffFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open delayOff file error" << endl;
            return -5;
        }
        sprintf(delayBuf, "%d", delayOff);
        n = write(fd, delayBuf, strlen(delayBuf));
        close(fd);
        return 0; 
    }

    int Led::stopBlink()
    {
        int fd;
        int n;

        if(access(this->ledDelayOnFile.c_str(), F_OK) != 0)
        {
            cout << "led not blinking" << endl;
            return 0;
        }

        fd = open(this->ledTriggerFile.c_str(), O_WRONLY);
        if(fd < 0)
        {
            cout << "open trigger file error" << endl;
            return -1;
        }
        n = write(fd, "none", 4);
        close(fd);
        if(n != 4)
        {
            cout << "write timer to trigger err" << endl;
            return -2;
        }

        this->lightOff();

        return 0;

    }

}