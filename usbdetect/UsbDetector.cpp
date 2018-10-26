#include <iostream>
#include <algorithm>

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
    #include <sys/socket.h>  
    #include <linux/netlink.h>  
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
        this->reportFlag = 0;

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
            
            if(buf[strlen(buf) - 1] == '\n')
            {
                buf[strlen(buf) - 1] = '\0';
            }

            string str(buf);
            #if DEBUG_PRINT > 0
            cout << "read from ls "<<str <<endl;
            #endif
            cout << "str size: "<< str.size() <<endl;            
               
            this->parseString(str, true);
        }

        UsbDetector::reportDeviceChange();

    }
    

    void* UsbDetector::threadFunc(void *arg)
    {
        UsbDetector *detetcor = (UsbDetector *)arg;
        struct sockaddr_nl client;
        struct timeval tv;
        int sockid, recvlen, ret;
        fd_set fds, tmp_fds;
        int bufferSize = 1024;
        char buf[bufferSize];

        
        cout << "usb detector thread start ..."<<endl;
        detetcor->setReportFlag(0);

        sockid = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
        client.nl_family = AF_NETLINK;
        client.nl_pid = getpid();
        client.nl_groups = 1;

        setsockopt(sockid, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
        bind(sockid, (struct sockaddr *) &client, sizeof(client));

        FD_ZERO(&fds);
        FD_SET(sockid, &fds);

        while(UsbDetector::execFlag)
        {
            tmp_fds = fds;
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            ret = select(sockid + 1, &tmp_fds, NULL, NULL, &tv);
            if(ret < 0)
            {
                cout << "usb detector select err" << endl;
                break;
            }

            if(ret == 0)
            {
                if(detetcor->getReportFlag())
                {
                    detetcor->reportDeviceChange();
                    detetcor->setReportFlag(0);
                }

                continue;
            }
                
            recvlen = recv(sockid, buf, sizeof(buf), 0);
            if(recvlen > 0)
            {
                string str(buf);
                if(str.find("add") == 0)
                {
                    cout <<"add " << str <<endl;
                    detetcor->parseString(str, true);
                }
                else if(str.find("remove") == 0)
                {
                     cout <<"remove " << str <<endl;
                     detetcor->parseString(str, false);
                }                
            }
        }

        
        detetcor->reportDeviceChange();
        cout << "usb detector thread exit" << endl;

    }


    pthread_t UsbDetector::startMonitor()
    {
        pthread_t tid;
        int ret;

        ret = pthread_create( &tid, NULL, UsbDetector::threadFunc, this);
        if(ret != 0)
        {
            cout << "usb detector create thread error" << endl;
            return 0;
        }
        this->monitorThreadId = tid;
        return tid;       
    }

    void UsbDetector::stopMonitor()
    {
        UsbDetector::execFlag = 0;
    }

    void UsbDetector::reportDeviceChange()
    {
        #if DEBUG_PRINT > 0

        vector<string>::iterator it;
        cout << "*********************************" <<endl;
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

    void UsbDetector::parseString(string str, bool add)
    {
        size_t pos;
        vector<string>::iterator it;
        if(str.find(this->inputHostAddr, 0) != string::npos)  //input usb
        {
            if(add)
            {
                if((pos = str.rfind("sd")) != string::npos)
                {
                    cout <<"in usb pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    if((it = find(UsbDetector::inputUsbDevice.begin(), UsbDetector::inputUsbDevice.end(), devName)) 
                            == UsbDetector::inputUsbDevice.end())
                    {
                        UsbDetector::inputUsbDevice.push_back(devName);
                        this->setReportFlag(1);
                    } 
                }   
            }
            else
            {
                if((pos = str.find(this->inputHostAddr)) != string::npos)
                {
                    if(!UsbDetector::inputUsbDevice.empty())
                    {
                        UsbDetector::inputUsbDevice.clear();
                        this->setReportFlag(1);
                    }                    
                }
            }
                           
        }       

        if(str.find(this->outputHostAddr, 0) != string::npos)  //output usb
        {
            if(add)
            {
                if((pos = str.rfind("sd")) != string::npos)
                {
                    cout <<"out usb pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    if((it = find(UsbDetector::outputUsbDevice.begin(), UsbDetector::outputUsbDevice.end(), devName)) 
                            == UsbDetector::outputUsbDevice.end())
                    {
                        UsbDetector::outputUsbDevice.push_back(devName);
                        this->setReportFlag(1);
                    }
                
                }  
            }
            else
            {
                if((pos = str.find(this->outputHostAddr)) != string::npos)
                {
                    if(!UsbDetector::outputUsbDevice.empty())
                    {
                        UsbDetector::outputUsbDevice.clear();
                        this->setReportFlag(1);
                    }                    
                }
            }  
        }         

        if(str.find(this->sdcardHostAddr, 0) != string::npos)  //output usb
        {
            if(add)
            {
                if((pos = str.rfind("mmcblk")) != string::npos)
                {
                    cout <<"sd card pos :" << pos <<endl;
                    string devName = str.substr(pos, str.size());
                    if((it = find(UsbDetector::sdcardDevice.begin(), UsbDetector::sdcardDevice.end(), devName)) 
                            == UsbDetector::sdcardDevice.end())
                    {
                        UsbDetector::sdcardDevice.push_back(devName);  
                        this->setReportFlag(1);        
                    }
                
                } 
            }
            else
            {
                if((pos = str.find(this->sdcardHostAddr)) != string::npos)
                {
                    if(!UsbDetector::sdcardDevice.empty())
                    {
                        UsbDetector::sdcardDevice.clear();
                        this->setReportFlag(1);
                    }                    
                }
            }
                  
        }

    }

    int UsbDetector::getReportFlag()
    {
        return this->reportFlag;
    }
    void UsbDetector::setReportFlag(int flag)
    {
        this->reportFlag = flag;
    }
}