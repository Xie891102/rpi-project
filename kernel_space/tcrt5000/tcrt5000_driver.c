// TRCT5000 裝置驅動層 Device Drivers

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "pin_mapping.h"
#include "tcrt5000_hal.h"

static int major;		// 定義全域major number


// ---------- 檔案處理 -------------


// 開啟檔案 open()
static int tcrt5000_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "tcrt5000: device opened\n");
	return 0;
}


// 讀取檔案 read()
static ssize_t tcrt5000_read(struct file *file, char __user *buf, size_t count, loff_t *ppos ){
	
	// 1.讀取感測器(左中右)
	int left = read_gpio(TCRT5000_LEFT);
	int middle = read_gpio(TCRT5000_MIDDLE);
	int right = read_gpio(TCRT5000_RIGHT);

	printk(KERN_INFO "TCRT5000 Read -Left: %d, -Middle: %d, -Right: %d", left, middle, right);

	// 2.封裝成字串，例 "0 1 0\n"
	char msg[20];
	int len = snprintf(msg, sizeof(msg), "%d %d %d\n", left, middle, right);

	// 3.避免讀取重複 => 第一次讀取後 ppos > 0 ，返回0
	if(*ppos > 0 || count < len) return 0;
	
	// 4.複製到user space(cat /dev/trct5000時顯示)
	if(copy_to_user(buf, msg, len)) return -EFAULT;

	*ppos = len;	// 更新讀取位置
	return len;	// 回傳實際讀到的字元數

}	


// 關閉裝置 release()
static int tcrt5000_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "tcrt5000: device closed\n");
	return 0;
}

 
// 結構體struct 上層指令對應到這裡的自訂函數
static struct file_operations fops = {
	.owner = THIS_MODULE,			// 防止模組卸載時，仍有操作
	.open = tcrt5000_open,
	.read = tcrt5000_read,
	.release = tcrt5000_release,
};


// ------------ 初始化 -------------

// 註冊device，取得major number
static int __init tcrt5000_driver_init(void){
	
	// 0.註冊字元裝置 0 => kernel自動分配
	major = register_chrdev(0, "tcrt5000", &fops);

	// 如果沒有數字 代表失敗
	if(major < 0){
		printk(KERN_ALERT "Register char device failed\n");
		return major;
	}
	
	// 成功註冊
	printk(KERN_INFO "TCRT5000 driver loaded, major number: %d\n", major);
	return 0;
}


// 卸載 解除註冊 device
static void __exit tcrt5000_driver_exit(void){
	
	// 解除註冊
	unregister_chrdev(major, "tcrt5000");
	printk(KERN_INFO "TCRT5000 driver unloaded\n");
}

	


// 設定 module 進入點、退出點
module_init(tcrt5000_driver_init); 	// insmod 時呼叫
module_exit(tcrt5000_driver_exit);	// rmmod 時呼叫


// 設定 module 資訊
MODULE_LICENSE("GPL");			// 授權GPL，避免被視為私有模組
MODULE_AUTHOR("XIE");			// 作者
MODULE_DESCRIPTION("TCRT5000 Device Driver"); 	// 簡短說明



