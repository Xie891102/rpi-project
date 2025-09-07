// TCRT5000 循跡感測器 硬體抽象層(HAL)
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include "gpio_pins.h"

// 初始化
static int __init tcrt5000_init(void){
	
	int req;

	// 註冊左邊循跡
	req = gpio_request(TCRT5000_LEFT, "TCRT5000_LEFT");
	if(req) return req;
	gpio_direction_input(TCRT5000_LEFT);

	// 註冊中間循跡
	req = gpio_request(TCRT5000_MIDDLE, "TCRT5000_MIDDLE");
	if(req) return req;
	gpio_direction_input(TCRT5000_MIDDLE);

	// 註冊右邊循跡
	req = gpio_request(TCRT5000_RIGHT, "TCRT5000_RIGHT");
	if(req) return req;
	gpio_direction_input(TCRT5000_RIGHT);

	// 成功印出
	printk(KERN_INFO "TCRT5000 HAL initialized\n");
	return 0;
}


// 卸載
static void  __exit tcrt5000_exit(void){

	// 釋放資源
	gpio_free(TCRT5000_LEFT);
	gpio_free(TCRT5000_MIDDLE);
	gpio_free(TCRT5000_RIGHT);
	printk(KERN_INFO "TCRT5000 HAL exited\n");
}


// Kernel Module 進入點/退出點
module_init(tcrt5000_init);		// insmod tcrt5000_hal.ko 執行時調用的
module_exit(tcrt5000_exit);		// rmmod tcrt5000_hal 執行時調用的
				
// 授權與設定
MODULE_LICENSE("GPL");			   // 授權GPL，避免被認為是私有 
MODULE_AUTHOR("XIE");			   // 作者
MODULE_DESCRIPTION("TCRT5000 HAL Layer");  // 簡短說明


