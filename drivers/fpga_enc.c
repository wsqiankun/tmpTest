#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <asm/div64.h>
#include <linux/pfn.h>

#include "fpga_enc.h"

static struct enc_fpga_device *gfpga_enc = NULL;


static void genDataDummy(struct work_struct *work)
{
    struct enc_fpga_device *fpga_enc = container_of(work, struct enc_fpga_device, work);
    int i;
    int t;
    int n;
    
    for(i = 0; i < fpga_enc->gen_data_len; i++)
    {
        n = i;
        t = do_div(n, fpga_enc->seed_len);
        fpga_enc->bufs[1].buf[i] = fpga_enc->seed[t];
    }   

    spin_lock_irq(&fpga_enc->fpga_spin_lock);
    fpga_enc->status = STATUS_SET_BUF;     
    spin_unlock_irq(&fpga_enc->fpga_spin_lock);
}

static int fpga_misc_open (struct inode *inodep, struct file *filep)
{
    struct enc_fpga_device *fpga_enc = gfpga_enc;
    // int i = 0;
    printk(KERN_WARNING "fpga misc open, buf size = %d\n",fpga_enc->bufs[0].size);
    // mutex_lock(&fpga_enc->buf_lock);
    // for(i = 0; i < fpga_enc->bufs[0].size; i++)
    //     fpga_enc->bufs[0].buf[i] = do_div(i, 256);
    // mutex_unlock(&fpga_enc->buf_lock);
    return 0;
}


static int fpga_misc_release (struct inode *inodep, struct file *filep)
{
    struct enc_fpga_device *fpga_enc = gfpga_enc;
    printk(KERN_WARNING "fpga misc releases\n");
    fpga_enc->status = STATUS_IDLE;
    return 0;
}

static int fpga_misc_mmap (struct file *filep, struct vm_area_struct *vma)
{     
    struct enc_fpga_device *fpga_enc = gfpga_enc;
    // pgprot_t *vma_prot = &vma->vm_page_prot;

    printk(KERN_WARNING "fpga misc mmap\n");
    // vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);  


    vma->vm_flags |= VM_IO;
    // vma->vm_flags |= VM_RESERVED;
    if (remap_pfn_range(vma,vma->vm_start,
            virt_to_phys(fpga_enc->bufs[1].buf)>>PAGE_SHIFT,
            vma->vm_end - vma->vm_start, 
            vma->vm_page_prot))
        return  -EAGAIN;

    memcpy(fpga_enc->bufs[1].buf, "abcdefg", 7);
    return 0;
}

static long fpga_misc_ioctl (struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct enc_fpga_device *fpga_enc = gfpga_enc;
    struct fpga_seed seed; 
    int status = 0;
    int ret;
    switch(cmd)
    {
    case IOC_CMD_SET_SEED:        
        printk(KERN_WARNING"ioctl set seed....\n");
        spin_lock_irq(&fpga_enc->fpga_spin_lock);
        status = fpga_enc->status;
        spin_unlock_irq(&fpga_enc->fpga_spin_lock);

        if(status != STATUS_IDLE)
            return -EAGAIN;

        ret  =  copy_from_user(&seed, (unsigned char __user *)arg, sizeof(seed));
        if(ret < 0)
            return -EAGAIN;

        fpga_enc->seed_len = seed.seed_len > fpga_enc->max_seed_len ? fpga_enc->max_seed_len : seed.seed_len;
        ret = copy_from_user(fpga_enc->seed, (unsigned char __user *)seed.seed, fpga_enc->seed_len);  
        if(ret < 0)
            return -EAGAIN;
        break;
    case IOC_CMD_GEN_DATA:
        printk(KERN_WARNING"ioctl gen data....\n");
        spin_lock_irq(&fpga_enc->fpga_spin_lock);
        status = fpga_enc->status;
        if(status != STATUS_IDLE)
        {
            spin_unlock_irq(&fpga_enc->fpga_spin_lock);
            return -EAGAIN;
        }       
        fpga_enc->status = STATUS_GEN_DATA;     
        spin_unlock_irq(&fpga_enc->fpga_spin_lock);      

        fpga_enc->gen_data_len = arg > PAGE_SIZE ? PAGE_SIZE : arg;
        schedule_work(&fpga_enc->work);
        break;
    case IOC_CMD_SET_BUF:
        printk(KERN_WARNING"ioctl set buff....\n");
        spin_lock_irq(&fpga_enc->fpga_spin_lock);
        status = fpga_enc->status;
         if(status != STATUS_SET_BUF)
        {
            spin_unlock_irq(&fpga_enc->fpga_spin_lock);
            return -EAGAIN;
        }          
        fpga_enc->status = STATUS_IDLE;
        spin_unlock_irq(&fpga_enc->fpga_spin_lock); 
        memcpy(fpga_enc->bufs[0].buf, fpga_enc->bufs[1].buf, fpga_enc->gen_data_len);
        break;
    case IOC_CMD_RESET:
        fpga_enc->status = STATUS_IDLE;
        memset(fpga_enc->bufs[0].buf, 0, PAGE_SIZE);
        memset(fpga_enc->bufs[1].buf, 0, PAGE_SIZE);
    default:
        break;
    }
    printk(KERN_WARNING"ioctl exit..... \n");
    return 0;
}

static ssize_t fpga_misc_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos)
{
    struct enc_fpga_device *fpga_enc = gfpga_enc;
    int ret = 0;
    unsigned char *addr = fpga_enc->bufs[0].buf;
    printk(KERN_WARNING"write..... \n");
    
    ret = copy_from_user( addr, buf, count);  
    
    return count;
}



static struct file_operations fpga_misc_fops = {
    .owner = THIS_MODULE,
    .open = fpga_misc_open,
    .release = fpga_misc_release,
    .unlocked_ioctl = fpga_misc_ioctl,
    .write = fpga_misc_write,
    .mmap = fpga_misc_mmap
};

static struct miscdevice fpga_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &fpga_misc_fops,
    .name = "fpga-enc"
};

static const struct of_device_id enc_fpga_dt_ids[] = {
    {.compatible = "fpga,encryption"},
    {}
};


MODULE_DEVICE_TABLE(of, enc_fpga_dt_ids);


static int parse_dt(struct platform_device *pdev)
{
    struct enc_fpga_device *fpga_enc;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *of_id = of_match_device(enc_fpga_dt_ids, &pdev->dev);
    u32 data_io_num = 0;
    u32 freq;
    u32 seed_len = 0;
    int ret;
    int i;

    if(!of_id)
        return -1;


    ret = of_property_read_u32(np, "max-seed-length", &seed_len);
    if(ret < 0)
    {
        return -ENODATA;
    }  

    ret = of_property_read_u32(np, "frequency", &freq);
    if(ret < 0)
    {
        return -ENODATA;
    }    

    ret = of_property_read_u32(np, "data-gpio-num", &data_io_num);
    if(ret < 0)
    {
        return -ENODATA;
    }
    
    fpga_enc = kzalloc(sizeof(struct enc_fpga_device) + (data_io_num + OTHER_IO_NUM)*sizeof(u32), 
                GFP_KERNEL);
    if(!fpga_enc)
    {
        dev_err(&pdev->dev, "alloc enc_fpga_device error\n");
        goto fpga_enc_err;
    }
        
    fpga_enc->pdev = pdev;
    fpga_enc->frequency = freq;
    fpga_enc->n_data_io = data_io_num;
    fpga_enc->max_seed_len = seed_len;
    fpga_enc->seed_len = seed_len;

    for(i = 0; i < data_io_num; i++)
    {
        fpga_enc->gpios[i] = of_get_named_gpio(np, "data-gpios", i);
    }
    
    fpga_enc->gpios[data_io_num] = of_get_named_gpio(np, "cs-gpios", 0);
    fpga_enc->gpios[data_io_num + 1] = of_get_named_gpio(np, "clk-gpios", 0);

    fpga_enc->seed = kzalloc(sizeof(unsigned char)* fpga_enc->max_seed_len, GFP_KERNEL);
    if(!fpga_enc->seed)
    {
        dev_err(&pdev->dev, "alloc seed buf error\n");
        goto fpga_enc_seed_err;
    }


    for(i =0; i < 2; i++)
    {
        fpga_enc->bufs[i].size = PAGE_SIZE;
        fpga_enc->bufs[i].buf = (char *)get_zeroed_page(GFP_KERNEL);
        if(!fpga_enc->bufs[i].buf)
        {
            dev_err(&pdev->dev, "alloc enc_fpga_device  buf error\n");
            goto fpga_enc_mm_err;
        }
    }

    platform_set_drvdata(pdev, fpga_enc);          

    printk(KERN_WARNING" fpga_enc dt:\n \
                         buf_size = %d\n \
                         frequenct = %d\n \
                         n_data_gpio = %d\n \
                         gpios:\n",                
                         fpga_enc->bufs[0].size,
                         fpga_enc->frequency,
                         fpga_enc->n_data_io);

    for(i=0; i < fpga_enc->n_data_io + 2; i++)
    {
        printk(KERN_WARNING"   %d,\n", fpga_enc->gpios[i]);
    }
    
    return 0;

fpga_enc_mm_err:   
    for(i = 0; i < 2; i++)
    {
        if(fpga_enc->bufs[i].buf)
            free_page((unsigned long)fpga_enc->bufs[i].buf);
    }
fpga_enc_seed_err:
     kfree(fpga_enc);

fpga_enc_err:  
    return -ENOMEM;
    
}


static int enc_fpga_probe(struct platform_device *pdev)
{
    struct enc_fpga_device *fpga_enc;
    int ret;

    ret = parse_dt(pdev);
    if(ret < 0)
    {
        dev_err(&pdev->dev, "parse dt err\n");
        return -ENODATA;
    }

    fpga_enc = platform_get_drvdata(pdev);
    gfpga_enc = fpga_enc;
    INIT_WORK(&fpga_enc->work, genDataDummy);
    spin_lock_init(&fpga_enc->fpga_spin_lock);

    spin_lock_irq(&fpga_enc->fpga_spin_lock);
    fpga_enc->status = 0;
    spin_unlock_irq(&fpga_enc->fpga_spin_lock);    

    fpga_enc->miscdev = &fpga_misc_dev;

    ret = misc_register(&fpga_misc_dev);
    if(ret < 0)
    {
        dev_err(&pdev->dev, "register miscdevice error");
        return -ENODEV;
    }

    return  0;
}


static int enc_fpga_remove(struct platform_device *pdev)
{
    struct enc_fpga_device *fpga_enc;
    int i;
    fpga_enc = platform_get_drvdata(pdev);
    
    misc_deregister(fpga_enc->miscdev);
    cancel_work_sync(&fpga_enc->work);
        
    fpga_enc->pdev = NULL;
    fpga_enc->miscdev = NULL;

    if(fpga_enc->seed)
    {
        kfree(fpga_enc->seed);
    }

    for(i = 0; i < 2; i++)
    {
        if(fpga_enc->bufs[i].buf)
            free_page((unsigned long)fpga_enc->bufs[i].buf);
    }

    if(fpga_enc)
        kfree(fpga_enc);
    return 0;
}


struct platform_driver fpga_driver = {
    .driver = {
        .name = "enc-fpga",
        .of_match_table = of_match_ptr(enc_fpga_dt_ids),
    },
    .probe = enc_fpga_probe,
    .remove = enc_fpga_remove,
};

module_platform_driver(fpga_driver);

MODULE_AUTHOR("scs qiankun.zhang, ");
MODULE_DESCRIPTION("fpga control driver");
MODULE_LICENSE("GPL");

