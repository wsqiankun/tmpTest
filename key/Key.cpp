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

#include "Key.h"
#include "../common/common.h"

namespace zc55
{
    volatile int Key::execFlag = 1;
    KeyCB Key::keyCB = Key::keyCBDummy;

    Key::Key(string name)
    {
        this->keyName = name;
    }

    void Key::keyCBDummy(unsigned char keyCode, KeyStatus pressed)
    {
        printf("Key %d  %s\n",keyCode, (pressed == KEY_PRESSED ) ? "pressed" : "released");
    }

    void Key::setKeyCB(KeyCB cb)
    {
        Key::keyCB = cb;
    }

    pthread_t Key::startKeyMonitor()
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
                    this->eventName = string(name);
                    close(fd);
                    #if DEBUG_PRINT > 0
                    printf("buf:%s\n", name);
                    cout << "find gpio Key " << this->eventName << endl;
                    #endif
                    break;
                }
                close(fd);
            }
        }
        if( i >= 32)
        {
            cout << "can't find gpio-Key" << endl;
            return 0;
        }

        // 创建thread
        if(pthread_create(&tidp, NULL, Key::threadFunc, (void*)this->eventName.c_str()) != 0)
        {
            cout << "Key create thread error" << endl;
            return 0;
        }

        return tidp;

    }

    void Key::stopKeyMonitor()
    {
        Key::execFlag = 0;
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
        printf("gpio key event :%s\n", (char*)arg);
        #endif

        int fd = 0;
        fd_set rd, tmp_rd;
        struct timeval tv;
        struct input_event event;
        int maxfd = 0;

        FD_ZERO(&rd);        
        
        if((fd = open((char*)arg, O_RDONLY | O_NONBLOCK, 0)) < 0 )
        // if(fd = open((char*)arg, O_RDONLY | O_NONBLOCK, 0) < 0 )  wrong
        {
            cout << "open event file error" << endl;
             //TODO: report error to main thread

            return NULL;
        }
        cout << "input fd " << fd << endl;
        FD_SET(fd, &rd);
        maxfd =fd +1;
        int rc, err;
        
        while (Key::execFlag > 0)
        {
            tmp_rd = rd;
            tv.tv_sec = KEY_SELECT_DELAY_S;
            tv.tv_usec = KEY_SELECT_DELAY_US;
            err = select(maxfd, &tmp_rd, NULL, NULL, &tv);
            if(err == 0)
            {
                // #if DEBUG_PRINT >0
                #if 0
                cout << "key thread select timeout" << endl;
                #endif

                continue;
            }
            else if(err < 0)
            {
                cout << "key thread select error" << endl;
                break;
            }
            else
            {
                rc = read(fd, &event, sizeof(event));
                if(rc <= 0)
                {
                    continue;
                }


                if(event.type == EV_KEY)
                {
                    if (event.code > BTN_MISC)
                    {
                        // printf("Button %d %s\n", event.code & 0xff, event.value ? "press" : "release");
                         Key::keyCB( event.code & 0xff,  event.value ?  KEY_PRESSED : KEY_RELEASED);
                    }
                    else
                    {
                        // printf("Key %d (0x%x) %s\n", event.code & 0xff, event.code & 0xff, event.value ? "press" : "release");
                        //TODO: report key
                        Key::keyCB( event.code & 0xff,  event.value ?  KEY_PRESSED : KEY_RELEASED);
                    }
                }
             
                
            }


            
        }
        close(fd);
        #if DEBUG_PRINT > 0 
        cout << "event file close when thread closed" << endl;
        #endif
    }

    void Key::reportKey()
    {

    }

}
