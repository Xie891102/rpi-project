/**
 * hc_sr04_multi.c - Linux kernel module to support multiple HC-SR04 sensors
 * with user-space configurable GPIO trigger/echo pins via ioctl.
 * Supports up to 4 sensors, mapped as /dev/ultrasonic0 to /dev/ultrasonic3
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include "hc_sr_ioctl.h" 
#include "register_map.h" // 樹莓派 GPIO 的定義

#define DEVICE_NAME "ultrasonic"    // 裝置名稱前綴
#define MAX_DEVICES 4               // 最多支援 4 組感測器

// 定義每個感測器裝置的結構體
struct hc_sr04_dev {
    int index;         // 裝置索引 (第幾個感測器)
    int trigger_gpio;  // trigger 腳位 GPIO 編號
    int echo_gpio;     // echo 腳位 GPIO 編號
};

// 全域裝置陣列 & cdev 物件
static struct hc_sr04_dev devices[MAX_DEVICES];
static struct cdev cdevs[MAX_DEVICES];
static struct class *ultra_class;
static dev_t dev_number;

// GPIO 寄存器指標
static volatile uint32_t *PERIBase;
static volatile uint32_t *reg_GPFSEL0, *reg_GPFSEL1, *reg_GPSET0, *reg_GPCLR0, *reg_GPLEV0;

// open：裝置開啟時呼叫，把對應裝置結構指到 file->private_data
static int hc_sr04_open(struct inode *inode, struct file *file) {
    int minor = iminor(inode);          // 取得 minor number (代表第幾個裝置)
    file->private_data = &devices[minor];
    return 0;
}

// release：裝置關閉時呼叫，這邊不用做什麼
static int hc_sr04_release(struct inode *inode, struct file *file) {
    return 0;
}

// read：用來觸發測距並回傳距離
static ssize_t hc_sr04_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    struct hc_sr04_dev *dev = filp->private_data;
    ktime_t start, end;    // 記錄回波開始、結束時間
    s64 duration_us;       // 回波持續時間 (微秒)
    unsigned int distance_mm;  // 計算出來的距離 (mm)
    char outbuf[16];       // 輸出用 buffer

    u64 temp;
    int timeout = 1000000; // 等待超時用 (防止無限迴圈)

    // --- 觸發超音波脈波 ---
    // 先確保 trigger 低電位
    writel(readl(reg_GPCLR0) | (1 << dev->trigger_gpio), reg_GPCLR0);
    udelay(2); // 2 微秒
    // 產生 10 微秒高電位脈波
    writel(readl(reg_GPSET0) | (1 << dev->trigger_gpio), reg_GPSET0);
    udelay(10); // 10 微秒
    // 再拉回低電位
    writel(readl(reg_GPCLR0) | (1 << dev->trigger_gpio), reg_GPCLR0);

    // --- 等待 echo 高電位開始 ---
    while (((readl(reg_GPLEV0) >> dev->echo_gpio) & 0x1) == 0 && timeout--) cpu_relax();
    if (timeout <= 0) return -EIO;   // 超時沒等到，回傳錯誤
    start = ktime_get();             // 記錄回波開始時間

    timeout = 1000000;
    // --- 等待 echo 變低電位 (回波結束) ---
    while (((readl(reg_GPLEV0) >> dev->echo_gpio) & 0x1) == 1 && timeout--) cpu_relax();
    if (timeout <= 0) return -EIO;   // 超時沒等到，回傳錯誤
    end = ktime_get();               // 記錄回波結束時間

    // --- 計算距離 ---
    duration_us = ktime_to_us(ktime_sub(end, start)); // 時間差 (微秒)
    temp = duration_us * 340; // 聲速 340 m/s，單位換算：微秒*340
    do_div(temp, 2);          // 往返距離除以2
    do_div(temp, 1000);       // 轉為 mm
    distance_mm = (unsigned int)temp;

    // 距離以字串型式回傳
    snprintf(outbuf, sizeof(outbuf), "%u", distance_mm);
    return copy_to_user(buf, outbuf, strlen(outbuf)) ? -EFAULT : strlen(outbuf);
}

// 設定 GPIO 腳位模式 (輸入/輸出)
static void set_gpio_function(int gpio, int is_output) {
    volatile uint32_t *reg;
    uint32_t shift, val;

    // 根據 GPIO 編號選對寄存器
    if (gpio < 10) {
        reg = reg_GPFSEL0;
        shift = gpio * 3;
    } else {
        reg = reg_GPFSEL1;
        shift = (gpio - 10) * 3;
    }

    // 先清除原來模式
    val = readl(reg);
    val &= ~(0x7 << shift);
    if (is_output) {
        val |= (0x1 << shift); // 設成 output
    }
    writel(val, reg);
}

// ioctl：可由 user-space 設定 trigger/echo 腳位
static long hc_sr04_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct hc_sr04_dev *dev = file->private_data;

    switch (cmd) {
        case HC_SR04_SET_TRIGGER:
            dev->trigger_gpio = arg;
            set_gpio_function(dev->trigger_gpio, 1); // 設成輸出
            break;
        case HC_SR04_SET_ECHO:
            dev->echo_gpio = arg;
            set_gpio_function(dev->echo_gpio, 0);    // 設成輸入
            break;
        default:
            return -EINVAL; // 不支援的命令
    }
    return 0;
}

// 檔案操作函式表
static struct file_operations hc_sr04_fops = {
    .owner = THIS_MODULE,
    .open = hc_sr04_open,
    .release = hc_sr04_release,
    .read = hc_sr04_read,
    .unlocked_ioctl = hc_sr04_ioctl,
};

// 模組初始化函式
static int __init hc_sr04_init(void) {
    int i;
    alloc_chrdev_region(&dev_number, 0, MAX_DEVICES, DEVICE_NAME); // 申請裝置編號
    ultra_class = class_create(DEVICE_NAME);                       // 創建裝置類別 (for /dev/)

    // 取得/對應 GPIO 寄存器的虛擬記憶體位址
    PERIBase = ioremap(GPIO_BASE, 0x100);
    reg_GPFSEL0 = PERIBase + (GPFSEL0 / 4);
    reg_GPFSEL1 = PERIBase + (GPFSEL1 / 4);
    reg_GPSET0  = PERIBase + (GPSET0 / 4);
    reg_GPCLR0  = PERIBase + (GPCLR0 / 4);
    reg_GPLEV0  = PERIBase + (GPLEV0 / 4);

    // 建立多個裝置節點
    for (i = 0; i < MAX_DEVICES; i++) {
        devices[i].index = i;
        devices[i].trigger_gpio = 3; // 預設 trigger
        devices[i].echo_gpio = 4;    // 預設 echo
        cdev_init(&cdevs[i], &hc_sr04_fops);
        cdevs[i].owner = THIS_MODULE;
        cdev_add(&cdevs[i], MKDEV(MAJOR(dev_number), i), 1);
        device_create(ultra_class, NULL, MKDEV(MAJOR(dev_number), i), NULL, "ultrasonic%d", i);
    }

    printk("[HC-SR04] Multi-device driver loaded. Major: %d\n", MAJOR(dev_number));
    return 0;
}

// 模組卸載函式
static void __exit hc_sr04_exit(void) {
    int i;
    for (i = 0; i < MAX_DEVICES; i++) {
        device_destroy(ultra_class, MKDEV(MAJOR(dev_number), i));
        cdev_del(&cdevs[i]);
    }
    class_destroy(ultra_class);
    unregister_chrdev_region(dev_number, MAX_DEVICES);
    iounmap(PERIBase); // 釋放 ioremap 的資源
    printk("[HC-SR04] Driver unloaded\n");
}

module_init(hc_sr04_init);
module_exit(hc_sr04_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BOEN");
