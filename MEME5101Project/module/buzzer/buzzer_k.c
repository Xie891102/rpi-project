#include <linux/version.h>
#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>
#include <linux/init.h>  
#include <linux/major.h>  
#include <linux/device.h>  
#include <linux/poll.h>  
#include <linux/io.h>  
#include <linux/delay.h>  
#include <linux/uaccess.h> /* for put_user */
#include "./include/buzzer_ioctl.h" 
#include "./include/rpi4_gpio.h"

#include <linux/cdev.h>
static struct cdev mycdev;

#define DEVICE_NAME "buzzer"
int Major;

uint32_t *PERIBase;
uint32_t *reg_GPFSEL0, *reg_GPFSEL1;
uint32_t *reg_GPSET0;
uint32_t *reg_GPCLR0;

// To do: declare a class for udev used
static struct class *charmodule_class; 

uint32_t fn;
uint32_t pin[10]={0,3,6,9,12,15,18,21,24,27};
int buzzer_on = 0;

module_param(buzzer_on, int, 0644);

static int buzzer_open(struct inode *inode, struct file *file)
{
    printk("buzzer_open() Nothing to do\n");

    return 0;
}

static int buzzer_release(struct inode *inode, struct file *file)
{
    printk("buzzer_release() Nothing to do\n");
    
    return 0;
}

/* Called when a process, which already opened the dev file, attempts to read from it. 
 * struct file *filp: the file operation
 * char *buffer: The buffer to fill with data
 * size_t length: The length of the buffer
 * loff_t *offset: Our offset in the file
 */
static ssize_t buzzer_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    char prbuf[80];
    int bytes_read = 0;

    sprintf(prbuf, "%d\0", buzzer_on);
    if ( copy_to_user(buffer, prbuf, strlen(prbuf)) !=0 ) {
        printk("copy_to_user() fail\n");
        return -EFAULT;
    }
    bytes_read++;
    return bytes_read;
}

/* Called when a process writes to dev file: echo "0" > /dev/buzzer0 */
static ssize_t buzzer_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    int bytes_written=0;
    char pwbuf[80];

    if ( copy_from_user(pwbuf, buff, len) !=0 ) {
        printk("buzzer_write(): len:%d\n", len);
        return -EFAULT;
    }
    sscanf(pwbuf,"%d",&buzzer_on);
    if ( buzzer_on )
        writel(readl(reg_GPSET0)|(0x01<<pin[0]), reg_GPSET0);
    else
        writel(readl(reg_GPCLR0)|(0x01<<pin[0]), reg_GPCLR0);

    bytes_written++;
    return bytes_written;
}

long buzzer_ioctl(struct file *file,unsigned int ioctl_num, unsigned long ioctl_param) {

    /* if ioctl_param ==0, should access the GPIO10 */
    if ( ioctl_param == 0)
	ioctl_param += 10;

    switch (ioctl_num) {
        case IOCTL_SET_BUZZER_ON:
		printk("IOCTL_SET_BUZZER_ON, %ld\n", ioctl_param);
		writel((0x01<<ioctl_param), reg_GPSET0);
#if 0 /* Use this line to turn the other GPIO low */
		//    0b 0010 0000
		// ~  0b 1101 1111
        	writel(~(0x01<<ioctl_param), reg_GPCLR0);
#endif
		buzzer_on = 1;
    		break;
        case IOCTL_SET_BUZZER_OFF:
		printk("IOCTL_SET_BUZZER_OFF %ld\n", ioctl_param);
		writel((0x01<<ioctl_param), reg_GPCLR0);
		buzzer_on = 0;
            break;
        case IOCTL_TOGGLE_BUZZER:
		printk("IOCTL_TOGGLE_LED %ld\n", ioctl_param);
		if ( buzzer_on )
			writel((0x01<<ioctl_param), reg_GPCLR0);
		else
			writel((0x01<<ioctl_param), reg_GPSET0);
		buzzer_on ^= 0;
            break;
        default:
            printk("Unknown IOCTL command\n");
    }

    return 0;
}

static struct file_operations fops = {
    .read = buzzer_read,
    .write = buzzer_write,
    .unlocked_ioctl = buzzer_ioctl,
    .open = buzzer_open,
    .release = buzzer_release,
};

int __init init_module(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    dev_t devno=0;

    if(Major) {
        if ( register_chrdev_region(MKDEV(Major,0), 1, DEVICE_NAME) < 0 ) {
            printk ("register_chrdev_region() fail\n");
           goto fail_register_chrdev;
        }
    }
    else {
        if (alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME) < 0) {
           printk ("alloc_chrdev_region() fail\n");
           goto fail_register_chrdev;
        }
        Major=MAJOR(devno);
    }
    cdev_init(&mycdev, &fops);
    mycdev.owner=THIS_MODULE;
    if(cdev_add(&mycdev, MKDEV(Major,0), 1)) {
        printk ("Error adding cdev\n");
           goto fail_cdev_add;
    }
#else
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk ("Registering the character device failed with %d\n", Major);
        return -1;
    }
#endif

    /* Create your own class under /sysfs */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0)
    charmodule_class = class_create(THIS_MODULE, "charmodule_class");
#else
    charmodule_class = class_create("charmodule_class"); 
#endif

    if(IS_ERR(charmodule_class)) {
      printk("Err: failed in creating class.\n");
      return -1;
    }

     /* Register your own device in sysfs, and this will cause udev to create corresponding device node */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,27)
    int i=0;
    device_create(charmodule_class, NULL, MKDEV(Major, i), NULL, "buzzer%d", i);
    i++;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
    device_create(charmodule_class, NULL, MKDEV(Major, 0), "buzzer"); 
#else
    class_simple_device_add( charmodule_class, MKDEV(Major, 0), NULL, "buzzer" );
#endif
    printk(KERN_ALERT"'mknod /dev/buzzer0 c %d 0'.\n", Major);

       // Use ioremap to map the physical address into virtual address
    PERIBase = ioremap(GPIO_BASE, 1 * 256);
    if ( PERIBase == NULL ) {
        printk("ioremap() fail\n");
	goto failed_ioremap;
    }
    reg_GPFSEL0=(PERIBase+GPFSEL0/4);
    reg_GPFSEL1=(PERIBase+GPFSEL1/4);
    reg_GPSET0=(PERIBase+GPSET0/4);
    reg_GPCLR0=(PERIBase+GPCLR0/4);

    /* Set the IO pin as Output
     *   Bit 29:27, 0b001, GPIO 9, output
     *   Bit 26:24, 0b001, GPIO 8, output
     *   Bit 23:21, 0b001, GPIO 7, output
     *   Bit 20:18, 0b001, GPIO 6, output
     *   Bit 17:15, 0b001, GPIO 5, output
     *   Bit 14:12, 0b001, GPIO 4, output
     *   Bit 11:9, 0b001, GPIO 3, output
     *   Bit 8:6, 0b001, GPIO 2, output
     *   Bit 5:3, 0b001, GPIO 1, output
     * */
    fn = (0x1 << pin[1]) | \
	 (0x1 << pin[2]) | \
	 (0x1 << pin[3]) | \
	 (0x1 << pin[4]) | \
	 (0x1 << pin[5]) | \
	 (0x1 << pin[6]) | \
	 (0x1 << pin[7]) | \
	 (0x1 << pin[8]) | \
	 (0x1 << pin[9]);

    writel(readl(reg_GPFSEL0)|fn, reg_GPFSEL0);
    printk("reg_GPFSEL0:%x\n", readl(reg_GPFSEL0));
    /* Set the IO pin as Output
     *   Bit 2:0, 0b001, GPIO 10, output
     */
    fn = (0x1 << pin[0]);
    writel(readl(reg_GPFSEL1)|fn, reg_GPFSEL1);
    printk("reg_GPFSEL1:%x\n", readl(reg_GPFSEL1));

    return 0;

failed_ioremap:
    device_destroy(charmodule_class, MKDEV(Major, 0)); 
    class_destroy(charmodule_class); 
fail_cdev_add:
fail_register_chrdev:
    return -1;
}

void __exit cleanup_module(void)
{
    // To do: Unmap the virtual address
    iounmap(PERIBase);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
    // Delete device node under /dev
    device_destroy(charmodule_class, MKDEV(Major, 0)); 
    // Delete class created by us
    class_destroy(charmodule_class); 
#else
    class_simple_device_remove(MKDEV(Major, 0));
    class_simple_destroy(charmodule_class);
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    cdev_del(&mycdev);
    unregister_chrdev_region(MKDEV(Major, 0), 1);
#else
    /* Unregister the device */
    unregister_chrdev(Major, DEVICE_NAME);
#endif
    printk("unregister_chrdev\n");

}

MODULE_LICENSE ("GPL");
