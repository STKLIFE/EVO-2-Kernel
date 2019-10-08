#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
/* #include <linux/xlog.h> */
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>

//#include <mach/mt_boot.h>
#include <mt-plat/mt_boot.h>
#include <linux/rtc.h>

#ifndef CONFIG_MTK_LEGACY
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#endif

#define maxlenth  255

typedef struct {
	//int id;
	char name[maxlenth + 1];
}JZHKHWINFO_DATA;

#define  JZHKHWINFO   0x90
#define  JZHKLCMIC          _IOR(JZHKHWINFO,0,JZHKHWINFO_DATA)
#define  JZHKTPIC           _IOR(JZHKHWINFO,1,JZHKHWINFO_DATA)
#define  JZHKBACKCAMERAIC   _IOR(JZHKHWINFO,2,JZHKHWINFO_DATA)
#define  JZHKFRONTCAMERAIC  _IOR(JZHKHWINFO,3,JZHKHWINFO_DATA)
//add for tee check begin
#define  JZHK_GOOGLE_KEY  _IOR(JZHKHWINFO,4,JZHKHWINFO_DATA)
#define  JZHK_TEE_KEY     _IOR(JZHKHWINFO,5,JZHKHWINFO_DATA)

//add for main flashlight by liuhui 20181206
#define  JZHK_FLASHLIGHT_TEST     _IOW(JZHKHWINFO,6,JZHKHWINFO_DATA)


//add for tee check end
#define ALLOC_DEVNO

static int debug_enable = 1;
#define jzhkhwinfo_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

// Definition
// device name and major number 
#define jzhkhwinfo_DEVNAME            "jzhkhwinfo"

extern char mtkfb_lcm_name[256];
extern char mtkfb_tp_name[256];
extern char mtkfb_frontcam_name[256];
extern char mtkfb_backcam_name[256];

static JZHKHWINFO_DATA curlcmic,curtpic,curbackcameraic,curfrontcameraic;
extern void mt_set_flash_light_on_for_false_led(int enable);

static int jzhkhwinfo_open(struct inode *inode, struct file *file)
{	
	jzhkhwinfo_DEBUG("frank jzhkhwinfo_open\n");
	return 0;
}

ssize_t jzhkhwinfo_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
       return 0;
}

static long jzhkhwinfo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret=0;
	jzhkhwinfo_DEBUG("jzhkhwinfo_ioctl cmd =%d\n",cmd);
	switch (cmd)
	{
		case JZHKLCMIC :
		{			

			jzhkhwinfo_DEBUG("frank mtkfb_lcm_name[0]=%c,mtkfb_lcm_name[1]=%c,mtkfb_lcm_name=%s\n",mtkfb_lcm_name[0],mtkfb_lcm_name[1],mtkfb_lcm_name);
			strcpy(curlcmic.name,mtkfb_lcm_name);			
			if (copy_to_user((JZHKHWINFO_DATA __user *) arg, &curlcmic, sizeof(JZHKHWINFO_DATA)))
			{
				jzhkhwinfo_DEBUG("frank lcm ic copy_to_user err\n");
				ret = -1;
			}
			break;
		}

		case JZHKTPIC :
		{
			strcpy(curtpic.name,mtkfb_tp_name);
			if (copy_to_user((JZHKHWINFO_DATA __user *) arg, &curtpic, sizeof(JZHKHWINFO_DATA)))	
			{
				jzhkhwinfo_DEBUG("frank tp ic copy_from_user err\n");
				ret = -1;
			}
			break;
		}

		case JZHKBACKCAMERAIC :
		{
			jzhkhwinfo_DEBUG("frank mtkfb_backcam_name=%s\n",mtkfb_backcam_name);
			strcpy(curbackcameraic.name,mtkfb_backcam_name);
			if (copy_to_user((JZHKHWINFO_DATA __user *) arg, &curbackcameraic, sizeof(JZHKHWINFO_DATA))) 
			{
				jzhkhwinfo_DEBUG("frank back camera ic copy_from_user err\n");
				ret = -1;
			}
			break;
		}
		
		case JZHKFRONTCAMERAIC :
		{
			jzhkhwinfo_DEBUG("frank mtkfb_backcam_name=%s\n",mtkfb_frontcam_name);
			strcpy(curfrontcameraic.name,mtkfb_frontcam_name);	
			if (copy_to_user((JZHKHWINFO_DATA __user *) arg, &curfrontcameraic, sizeof(JZHKHWINFO_DATA))) 
			{
				jzhkhwinfo_DEBUG("frank front camera ic copy_from_user err\n");
				ret = -1;
			}
			break;
		}
		//add for main flashlight by liuhui 20181206
		case JZHK_FLASHLIGHT_TEST :
		{
			jzhkhwinfo_DEBUG("JZHK_FLASHLIGHT_TEST\n");
			mt_set_flash_light_on_for_false_led(1);
			break;
		}
	}
	return 0;
}

static const struct file_operations jzhkhwinfo_fops = {
    .owner   = THIS_MODULE,
	.unlocked_ioctl  = jzhkhwinfo_ioctl,
	.open	= jzhkhwinfo_open,
	.read	 = jzhkhwinfo_read,	
};

static struct class *jzhkhwinfo_class = NULL;
static struct device *jzhkhwinfo_device = NULL;
static dev_t jzhkhwinfo_devno;
static struct cdev jzhkhwinfo_cdev;

static int __init jzhkhwinfo_probe(struct platform_device *pdev)
{
	int ret = 0, err = 0;

#ifdef ALLOC_DEVNO
    ret = alloc_chrdev_region(&jzhkhwinfo_devno, 0, 1, jzhkhwinfo_DEVNAME);
    if (ret) {
        jzhkhwinfo_DEBUG("frank alloc_chrdev_region fail: %d\n", ret);
        goto jzhkhwinfo_probe_error;
    } else {
        jzhkhwinfo_DEBUG("frank major: %d, minor: %d\n", MAJOR(jzhkhwinfo_devno), MINOR(jzhkhwinfo_devno));
    }
    cdev_init(&jzhkhwinfo_cdev, &jzhkhwinfo_fops);
    jzhkhwinfo_cdev.owner = THIS_MODULE;
    err = cdev_add(&jzhkhwinfo_cdev, jzhkhwinfo_devno, 1);
    if (err) {
        jzhkhwinfo_DEBUG("frank cdev_add fail: %d\n", err);
        goto jzhkhwinfo_probe_error;
    }
#else
    #define jzhkhwinfo_MAJOR 242
    ret = register_chrdev(jzhkhwinfo_MAJOR, jzhkhwinfo_DEVNAME, &jzhkhwinfo_fops);
    if (ret != 0) {
        jzhkhwinfo_DEBUG("frank Unable to register chardev on major=%d (%d)\n", jzhkhwinfo_MAJOR, ret);
        return ret;
    }
    jzhkhwinfo_devno = MKDEV(jzhkhwinfo_MAJOR, 0);
#endif

	jzhkhwinfo_class = class_create(THIS_MODULE, "jzhkhwinfo_drv");
    if (IS_ERR(jzhkhwinfo_class)) {
        jzhkhwinfo_DEBUG("frank Unable to create class, err = %d\n", (int)PTR_ERR(jzhkhwinfo_class));
        goto jzhkhwinfo_probe_error;
    }

    jzhkhwinfo_device = device_create(jzhkhwinfo_class, NULL, jzhkhwinfo_devno, NULL, jzhkhwinfo_DEVNAME);

    if(NULL == jzhkhwinfo_device){
        jzhkhwinfo_DEBUG("frank device_create fail\n");
        goto jzhkhwinfo_probe_error;
    }
    jzhkhwinfo_DEBUG("frank jzhkhwinfo Done\n");
    return 0;

jzhkhwinfo_probe_error:
#ifdef ALLOC_DEVNO
    if (err == 0)
        cdev_del(&jzhkhwinfo_cdev);
    if (ret == 0)
        unregister_chrdev_region(jzhkhwinfo_devno, 1);
#else
    if (ret == 0)
        unregister_chrdev(MAJOR(jzhkhwinfo_devno), jzhkhwinfo_DEVNAME);
#endif
    return -1;
}

static int jzhkhwinfo_remove(struct platform_device *dev)
{

#ifdef ALLOC_DEVNO
    cdev_del(&jzhkhwinfo_cdev);
    unregister_chrdev_region(jzhkhwinfo_devno, 1);
#else
    unregister_chrdev(MAJOR(jzhkhwinfo_devno), jzhkhwinfo_DEVNAME);
#endif
    device_destroy(jzhkhwinfo_class, jzhkhwinfo_devno);
    class_destroy(jzhkhwinfo_class);

    jzhkhwinfo_DEBUG("[jzhkhwinfo_remove] Done\n");
    return 0;
}
	
static struct platform_driver jzhkhwinfo_driver = {
	.driver		= {
		.name	= "jzhkhwinfo",
		.owner	= THIS_MODULE,
	},
	.probe		= jzhkhwinfo_probe,
	.remove		= jzhkhwinfo_remove,
};

struct platform_device jzhkhwinfo_device1 = {
	.name = "jzhkhwinfo",
	.id = -1,
};
	
static int __init jzhkhwinfo_init(void)
{
	int ret;

	jzhkhwinfo_DEBUG("frank jzhkhwinfo_init\n");
	ret = platform_device_register(&jzhkhwinfo_device1);
	if (ret)
		jzhkhwinfo_DEBUG("[jzhkhwinfo] frank jzhkhwinfo_device_init:dev:%d\n", ret);

	ret = platform_driver_register(&jzhkhwinfo_driver);

	if (ret)
	{
		jzhkhwinfo_DEBUG("[jzhkhwinfo]jzhkhwinfo_init:drv:E%d\n", ret);
		return ret;
	}

	return ret;
}

static void __exit jzhkhwinfo_exit(void)
{
	platform_driver_unregister(&jzhkhwinfo_driver);
}

module_param(debug_enable, int,0644);

module_init(jzhkhwinfo_init);
module_exit(jzhkhwinfo_exit);

MODULE_AUTHOR("JZHK Inc By frank");
MODULE_DESCRIPTION("JZHKHWINFO for MediaTek MT65xx chip");
MODULE_LICENSE("GPL");
MODULE_ALIAS("JZHKHWINFO");
