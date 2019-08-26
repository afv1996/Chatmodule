#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>

struct _chat_drv
{
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
}chat_drv;





int chat_init(void)
{
	int ret = 0;
	/*Allocate device number*/
	chat_drv.dev_num = 0;
	ret = alloc_chrdev_region(&chat_drv.dev_num,0 ,1, "chat_device");
	if (ret < 0)
	{
		printk(KERN_INFO " Failed to register device number dynamically\n");
		goto failed_register_devnum;
	}
	printk(KERN_INFO "Allocated device number (%d,%d)\n", MAJOR(chat_drv.dev_num),\
			                                      MINOR(chat_drv.dev_num));
        /*Create device file*/
        chat_drv.dev_class = class_create(THIS_MODULE, "class_chat_dev");
	if(chat_drv.dev_class == NULL)
	{
		printk(KERN_INFO "Failed to create a device class\n");
		goto failed_create_class;
	}
	chat_drv.dev = device_create(chat_drv.dev_class, NULL, chat_drv.dev_num, NULL,"chat_dev");
	if(IS_ERR(chat_drv.dev))
	{
		printk(KERN_INFO "Failed to create a device\n");
		goto failed_create_device;
	}
	printk(KERN_INFO "Initialize chat driver successfully\n");
	
	

	return 0;
	
	/*Failed action*/
	failed_create_class:
		unregister_chrdev_region(chat_drv.dev_num,1);
	failed_create_device:
		class_destroy(chat_drv.dev_class);
	failed_register_devnum:
		return ret;

}

void chat_exit(void)
{
	/*Free device number*/
	unregister_chrdev_region(chat_drv.dev_num,1);
	printk("good bye cpu\n");

	/*Delete all device file*/
	device_destroy(chat_drv.dev_class, chat_drv.dev_num);
	class_destroy(chat_drv.dev_class);
	

}

module_init(chat_init);
module_exit(chat_exit);
MODULE_AUTHOR("Nhat Anh");
MODULE_LICENSE("GPL");

