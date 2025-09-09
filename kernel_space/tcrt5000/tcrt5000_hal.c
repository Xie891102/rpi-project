// TCRT5000 循跡感測器 硬體抽象層(HAL)
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>		// insmod, readl, writel
#include "pin_mapping.h"	// GPIO 暫存器 offset 定義(datasheet)
#include "register_map.h"	// GPIO 暫存器 專案定義


// kernel 的虛擬位址
static void __iomem *gpio_base;


// 設定	GPIO16 為輸入腳位
static void gpio16_set_input(void){
	unsigned int val = readl(gpio_base + GPIO16_FSEL);	// 讀出 GPFSEL1
       	val &= ~(0x7 << GPIO16_BIT_SHIFT);		// 清除對應 3 BITS	
	writel(val, gpio_base + GPIO16_FSEL);	// 回寫 => 設為input
}

// 設定 GPIO17 為輸入腳位
static void gpio17_set_input(void){
	unsigned int val = readl(gpio_base + GPIO17_FSEL);
	val &= ~(0x7 << GPIO17_BIT_SHIFT);
	writel(val, gpio_base + GPIO17_FSEL);
}

// 設定 GPIO18 為輸入腳位
static void gpio18_set_input(void){
	unsigned int val = readl(gpio_base + GPIO18_FSEL);
	val &= ~(0x7 << GPIO18_BIT_SHIFT);
	writel(val, gpio_base + GPIO18_FSEL);
}


// 模組初始化
static int __init tcrt5000_init(void){
	
	// 1. 對GPIO BASE 做ioremap 取虛擬位址
	gpio_base = ioremap(GPIO_BASE, 0xB0);
	if(!gpio_base) {
		printk(KERN_ERR "TCRT5000 HAL: ioremap failed\n");
		return -ENOMEM;
	}	

	// 2. 設定 3 個感測器的 GPIO 腳位為輸入模式
	gpio16_set_input();	// 左
	gpio17_set_input();	// 中
	gpio18_set_input();	// 右
				
	// 3. 初始化完成
	printk(KERN_INFO "TCRT5000 HAL initialized\n");
	return 0;
				 
}


// 模組卸載
static void  __exit tcrt5000_exit(void){

	// 釋放資源
	iounmap(gpio_base);

	// 卸載成功
	printk(KERN_INFO "TCRT5000 HAL exited\n");
}


// Kernel Module 進入點/退出點
module_init(tcrt5000_init);		// insmod tcrt5000_hal.ko 執行時調用的
module_exit(tcrt5000_exit);		// rmmod tcrt5000_hal 執行時調用的
	

// Device Driver 要使用的讀取 GPIO 腳位的值
int read_gpio(int gpio){
	
	unsigned int val, bit;

	switch(gpio){
		case TCRT5000_LEFT:
			val = readl(gpio_base + GPIO16_FSEL);	// 讀取 GPIO16 腳位的值  
			bit = (val >> GPIO16_BIT_SHIFT) & 0X1;	// 將目標位元往右移，取最低位元作為 input 值
			return bit;

		case TCRT5000_MIDDLE:
			val = readl(gpio_base + GPIO17_FSEL);
			bit = (val >> GPIO17_BIT_SHIFT) & 0X1;
			return bit;

		case TCRT5000_RIGHT:
			val = readl(gpio_base + GPIO18_FSEL);
			bit = (val >> GPIO18_BIT_SHIFT) & 0X1;
			return bit;

		default:
			// 非法編號 回傳 -1
			return -1;
	}
}

// 匯出給其他 kernel module 使用
EXPORT_SYMBOL(read_gpio);


// 授權與設定
MODULE_LICENSE("GPL");			   // 授權GPL，避免被認為是私有 
MODULE_AUTHOR("XIE");			   // 作者
MODULE_DESCRIPTION("TCRT5000 HAL Layer");  // 簡短說明


