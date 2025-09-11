// TRCT5000 裝置驅動層 Device Drivers

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include "pin_mapping.h"
#include "tcrt5000_hal.h"
	
// ---------- 全域變數 ------------
static dev_t dev;		// 儲存分配到的 major/minor 
static struct cdev c_dev;	// 字元裝置的結構體
static struct class *cl;	// class 用來 /dev 創建節點



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


// 定義 devnode callback，設定 /dev/tcrt5000 權限 改成0666
static char *tcrt5000_devnode(const struct device *dev, umode_t *mode){
	if(mode) *mode = 0666;	// 所有人都可以寫入
	return NULL;		// 預設裝置名稱
}


// ------------ 初始化 -------------

// 註冊device，取得major number
static int __init tcrt5000_driver_init(void){
	
	int ret;
	printk(KERN_INFO "TCRT5000: Initializing driver...\n");

	// 1.分配 major + minor，讓Kernel自動分配裝置號碼
	ret = alloc_chrdev_region(&dev, 0, 1, "tcrt5000");
	if(ret < 0){
		printk(KERN_ALERT "TCRT5000: Failed to allocate char device region\n");
		return ret;
	}
	printk(KERN_INFO "TCRT5000 major number allocated= %d\n", MAJOR(dev));
	printk(KERN_INFO "TCRT5000 minor number= %d\n"	, MINOR(dev));


	// 2.初始化 cdev 結構體，並連結到 file operations
	cdev_init(&c_dev, &fops);
	c_dev.owner = THIS_MODULE;			// 防止卸載時 還有再操作


	// 3.正式註冊裝置，將 cdev 加入 kernel
	ret = cdev_add(&c_dev, dev, 1);			// 裝置的數量 1
	if(ret < 0){
		unregister_chrdev_region(dev, 1);	// 取消註冊 釋放資源
		printk(KERN_ALERT "TCRT5000: Failed to add cdev\n");
		return ret;
	}	


	// 4.創建 class，用於 /dev 下自動生成節點
	cl = class_create("tcrt5000_class");
	if(IS_ERR(cl)){					// 如果有錯誤
		cdev_del(&c_dev);			// 先刪除 cdev
		unregister_chrdev_region(dev, 1);	// 釋放 major/minor
		printk(KERN_ALERT "TCRT5000: Failed to create class\n");
		return PTR_ERR(cl);
	}

	// 指定 devnode callback
	cl->devnode = tcrt5000_devnode;


	// 5.正式創建 /dev/tcrt5000 節點，裝置的入口 讓應用層可以直接用
	device_create(cl, NULL, dev, NULL, "tcrt5000");


	// 成功註冊
	printk(KERN_INFO "TCRT5000 driver loaded successfully!\n");
	return 0;
}


// 卸載 解除註冊 device
static void __exit tcrt5000_driver_exit(void){
	
	// 1.刪除 device node
	device_destroy(cl, dev);

	// 2.刪除 class
	class_destroy(cl);
	
	// 3.刪除 cdev 結構體
	cdev_del(&c_dev);

	// 4.釋放資源 major/minor
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO "TCRT5000 driver unloaded successfully\n");
}


// 設定 module 進入點、退出點
module_init(tcrt5000_driver_init); 	// insmod 時呼叫
module_exit(tcrt5000_driver_exit);	// rmmod 時呼叫


// 設定 module 資訊
MODULE_LICENSE("GPL");			// 授權GPL，避免被視為私有模組
MODULE_AUTHOR("XIE");			// 作者
MODULE_DESCRIPTION("TCRT5000 Device Driver connected auto Device Node"); 	// 簡短說明



