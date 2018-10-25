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
    #include <pthread.h>
}

#include "Key.h"
#include "../common/common.h"

namespace zc55
{
    volatile int Key::threadFlag = 1;

    int Key::keyInit()
    {
        int fd;
        int i;
        char name[64];
        char buf[64];
        pthread_t tidp;

        for(i = 0; i < 32; i++)
        {
            sprintf(name, "/dev/input/event%d", i);
            if((fd = open(name, O_RDONLY)) >= 0)
            {
                ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
                if(strncmp(this->keyName.c_str(), buf, strlen(this->keyName.c_str())) == 0)
                {
                    this->eventName = string(buf);
                    close(fd);
                    #if DEBUG_PRINT > 0
                    cout << "find gpio Key" << this->eventName << endl;
                    #endif
                    break;
                }
                close(fd);
            }
        }
        if( i >= 32)
        {
            cout << "can't find gpio-Key" << endl;
            return -1;
        }

        // 创建thread
        if(pthread_create(&tidp, NULL, Key::threadFunc, (void*)this->eventName.c_str()) != 0)
        {
            cout << "Key create thread error" << endl;
            return -2;
        }

        return tidp;

    }

    void* Key::threadFunc(void *arg)
    {
        if(arg == NULL)
        {
            cout << "arg null pointer" << endl;
            //TODO: report error to main thread

            return NULL;
        }          

        cout << "gpio key thread start..."<< endl;
        #if DEBUG_PRINT > 0 
        printf("gpio key event :%s", (char*)arg);
        #endif

        int fd;
        
        if(fd = open((char*)arg, O_RDWR, 0) <=0 )
        {
            cout << "open event file error" << endl;
             //TODO: report error to main thread

            return NULL;
        }

        int rc;
        struct input_event event;
        while (((rc = read(fd, &event, sizeof(event))) > 0) &&  (Key::threadFlag >= 0))
        {
            if(event.type == EV_KEY)
            {
                 if (event.code > BTN_MISC)
                {
                    printf("Button %d %s", event.code & 0xff, event.value ? "press" : "release");
                }
                else
                {
                    printf("Key %d (0x%x) %s", event.code & 0xff, event.code & 0xff, event.value ? "press" : "release");
                    //TODO: report key
                }
            }
        }
        close(fd);
        #if DEBUG_PRINT > 0 
        cout << "event file close thile thread closed" << endl;
        #endif
    }

    void Key::reportKey()
    {

    }

}
