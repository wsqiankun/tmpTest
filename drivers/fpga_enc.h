#ifndef __FPGA_ENC_H__
#define __FPGA_ENC_H__

#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/spinlock.h>


#define NETLINK_FPGA 22

#define OTHER_IO_NUM 2

#define IOC_CMD_SET_SEED 0xABCD01
#define IOC_CMD_GEN_DATA 0xABCD02
#define IOC_CMD_SET_BUF  0xABCD03
#define IOC_CMD_RESET  0xABCD04


#define STATUS_IDLE      0
#define STATUS_GEN_DATA  1
#define STATUS_SET_BUF   2


struct mm_data_buf{
    unsigned char *buf;
    int size;
};

struct fpga_seed{
    unsigned char *seed;
    int seed_len;
};

struct enc_fpga_device{
    struct miscdevice *miscdev;
    struct platform_device *pdev;
    spinlock_t fpga_spin_lock;
    struct work_struct work;

    int gen_data_len;
    struct mm_data_buf bufs[2];
    int status;

    int max_seed_len;  //seed-len = <n>;
    int seed_len;
    unsigned char *seed;

    int frequency; //frequency = <25000000>;
    int n_data_io; 
    int gpios[]; //data cs clk ...  data-gpios=<> ;cs-gpios=<>; clk-gpios=<>;
};

#endif
