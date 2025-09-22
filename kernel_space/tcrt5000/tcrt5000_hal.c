// TCRT5000 循跡感測器 硬體抽象層(HAL)
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>		// insmod, readl, writel
#include "pin_mapping.h"	// GPIO 暫存器 offset 定義(datasheet)
#include "register_map.h"	// GPIO 暫存器 專案定義
#include "tcrt5000_hal.h"	// 編譯器先看到有這個檔案存在，避免警告


// kernel 的虛擬位址
static void __iomem *gpio_base;


// 設定	GPIO16 為輸入腳位
static void gpio9_set_input(void){
	unsigned int val = readl(gpio_base + GPIO9_FSEL);	// 讀出 GPFSEL1
       	val &= ~(0x7 << GPIO9_BIT_SHIFT);		// 清除對應 3 BITS	
	writel(val, gpio_base + GPIO9_FSEL);	// 回寫 => 設為input
}

// 設定 GPIO10 為輸入腳位
static void gpio10_set_input(void){
	unsigned int val = readl(gpio_base + GPIO10_FSEL);
	val &= ~(0x7 << GPIO10_BIT_SHIFT);
	writel(val, gpio_base + GPIO10_FSEL);
}

// 設定 GPIO11 為輸入腳位
static void gpio11_set_input(void){
	unsigned int val = readl(gpio_base + GPIO11_FSEL);
	val &= ~(0x7 << GPIO11_BIT_SHIFT);
	writel(val, gpio_base + GPIO11_FSEL);
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
	gpio9_set_input();	// 左
	gpio10_set_input();	// 中
	gpio11_set_input();	// 右
				
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

	// GPIO 0~31 讀取 GPLEV0 暫存器的輸入值   GPIO 32~53 讀取 GPLEV0 暫存器的值  
	val = readl(gpio_base + GPLEV0);

	switch(gpio){
		case TCRT5000_LEFT:
			bit = (val >> 9) & 0X1;		// 取 GPIO9 位元	
			return bit;

		case TCRT5000_MIDDLE:
			bit = (val >> 10) & 0X1;	// 取 GPIO10 位元
			return bit;

		case TCRT5000_RIGHT:
			bit = (val >> 11) & 0X1;	// 取 GPIO11 位元
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


