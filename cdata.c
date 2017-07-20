#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define BUF_SIZE 	8
#define CDATA_MAJOR 121

struct cdata_t {
	char *buf;
	int idx;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
	struct cdata_t *cdata;

	printk(KERN_ALERT "cdata in open: filp = %p\n", filp);

	while (1)
	{
		schedule();
	}

	cdata = kmalloc(sizeof(*cdata), GFP_KERNEL);
	cdata->buf = kmalloc(BUF_SIZE, GFP_KERNEL);
	cdata->idx = 0;

	filp->private_data = (void*)cdata;

	return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
	struct cdata_t *cdata = (struct cdata_t*)filp->private_data;
	int idx = cdata->idx;

	cdata->buf[idx] = '\0';

	printk(KERN_ALERT "data buffer %s\n", cdata->buf);

	kfree(cdata->buf);
	kfree(cdata);

	printk(KERN_ALERT "cdata is close: filp = %p\n", filp);

	return 0;
}

static ssize_t cdata_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct cdata_t *cdata = (struct cdata_t*)file->private_data;
	int i = 0, idx = cdata->idx;

	for (i = 0; i < count; i++)
	{
		if (idx >= BUF_SIZE - 1)
		{
			current->state = TASK_UNINTERRUPTIBLE;
			schedule();
		}
		copy_from_user(&cdata->buf[idx], &buf[i], sizeof(*buf));
		idx++;
	}
	
	cdata->idx = idx;

	return 0;
}

static struct file_operations cdata_fops = {
	open:		cdata_open,
	release: 	cdata_close,
	write: 		cdata_write,
};
static struct miscdevice cdata_miscdev = {
	.minor	= 77,
	.name	= "cdata-misc",
	.fops	= &cdata_fops,
};

static int cdata_plat_probe(struct platform_deivce *pdev)
{
	int res = -1;

	res = misc_register(&cdata_miscdev);
	if (res < 0)
	{
		printk("register failed\n");
		return -1;
	}
	printk("register success\n");

	return 0;
}
static int cdata_plat_remove(struct platform_device *pdev)
{
	misc_deregister(&cdata_miscdev);
	printk("unregister success\n");
}

static struct platform_driver cdata_driver = {
	.probe	 	= cdata_plat_probe,
	.remove 	= cdata_plat_remove,
	.driver 	= {
		.name 	= "cdata",
		.owner 	= THIS_MODULE,
	},
};

int cdata_init_module(void)
{
	return platform_driver_register(&cdata_driver);
}

void cdata_cleanup_module(void)
{
	platform_driver_unregister(&cdata_driver);
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
