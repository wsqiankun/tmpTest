#include<iostream>

extern "C"{
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/ioctl.h>
    #include <string.h>
}

#include "fpga.h"

using namespace std;
namespace zc55{

    Fpga::Fpga(string name):fd(-1)
    {
        this->fpgaName = name;
    }

    Fpga::~Fpga()
    {
        if(this->mapAddr)
        {
            munmap(this->mapAddr, sysconf(_SC_PAGE_SIZE));
        }

        if(this->fd > 0)
            close(this->fd);
    }

    unsigned char * Fpga::init()
    {
        unsigned char *start;
        this->fd = open(this->fpgaName.c_str(), O_RDWR);
        if(this->fd < 0)
        {
            cout<< "open fpga dev err" << this->fpgaName << endl;
            return NULL;
        }

        start = (unsigned char *)mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE , MAP_SHARED, this->fd, 0);

        this->mapAddr = start;
        return start;
    }

    int Fpga::setSeed(struct fpga_seed *seed)
    {
        int ret;

        ret =  ioctl(this->fd, IOC_CMD_SET_SEED, seed);
        if(ret < 0)
        {
            cout << "set seed error" << endl;
            return ret;
        }

        ret = ioctl(this->fd, IOC_CMD_GEN_DATA, sysconf(_SC_PAGE_SIZE));
        if(ret < 0)
        {
            cout << "gen data error" << endl;
            return ret;
        }

        return 0;
    }

    int Fpga::genData()
    {
        int ret;

        ret =  ioctl(this->fd, IOC_CMD_SET_BUF);
        if(ret < 0)
        {
            cout << "set seed error" << endl;
            return ret;
        }

        ret = ioctl(this->fd, IOC_CMD_GEN_DATA, sysconf(_SC_PAGE_SIZE));
        if(ret < 0)
        {
            cout << "gen data error" << endl;
            return ret;
        }

        return 0;
    }

    int Fpga::reset()
    {
        return ioctl(this->fd, IOC_CMD_RESET);
    }

}