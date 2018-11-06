#ifndef __FPGA_H__
#define __FPGA_H__

#include <string>
using namespace std;

namespace zc55{
#define IOC_CMD_SET_SEED 0xABCD01
#define IOC_CMD_GEN_DATA 0xABCD02
#define IOC_CMD_SET_BUF  0xABCD03
#define IOC_CMD_RESET  0xABCD04

struct fpga_seed{
    unsigned char *seed;
    int seed_len;
};


class Fpga{
public:
    Fpga(string name);
    ~Fpga();
    unsigned char * init();
    int genData();  //default one page 4k bytes size;
    int setSeed(struct fpga_seed *seed);
    int reset();

private:
    int fd;
    string fpgaName;
    void *mapAddr;

private:
};




}



#endif