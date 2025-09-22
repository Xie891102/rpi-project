#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/pwm.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "motor_gpio.h"

/* ===== 基本定義 ===== */
#define DEVICE_NAME "motor"
#define CLASS_NAME "motor_class"

/* 馬達設備結構體 */
struct l298n_dev {
    struct cdev cdev;                    // 字符設備結構
    struct class *class;                 // 設備類別
    dev_t dev_num;                       // 設備號碼
    void __iomem *gpio_base;             // GPIO暫存器基址
    unsigned int period_ns;              // PWM週期 (奈秒)
    unsigned int duty_ns;                // PWM duty cycle (奈秒)
    bool gpio_requested;                 // GPIO是否已申請
    bool pwm_configured;                 // PWM是否已配置
    int left_speed;                      // 左馬達速度 (0-100%)
    int right_speed;                     // 右馬達速度 (0-100%)
    struct device *device;               // 設備指標
    struct platform_device *pdev;       // 平台設備指標

    // GPIO descriptors
    struct gpio_desc *gpio_in1;
    struct gpio_desc *gpio_in2;
    struct gpio_desc *gpio_in3;
    struct gpio_desc *gpio_in4;
    
    struct pwm_device *pwm0;             // PWM0通道
    struct pwm_device *pwm1;             // PWM1通道
};

static struct l298n_dev *global_motor_dev = NULL;

/* ===== Platform Driver 函數 ===== */

static int motor_probe(struct platform_device *pdev)
{
    struct l298n_dev *ln;
    struct device *dev = &pdev->dev;
    int ret;

    dev_info(dev, "Motor driver probe started\n");

    ln = devm_kzalloc(dev, sizeof(*ln), GFP_KERNEL);
    if (!ln)
        return -ENOMEM;

    ln->pdev = pdev;
    platform_set_drvdata(pdev, ln);
    global_motor_dev = ln;

    if (IS_ERR(ln->pwm0)) {
        dev_err(dev, "failed to get pwm0: %ld\n", PTR_ERR(ln->pwm0));
        return PTR_ERR(ln->pwm0);
    }
    /* 從設備樹獲取PWM設備 */
    ln->pwm0 = devm_pwm_get(dev, "pwm0");
    if (IS_ERR(ln->pwm0)) {
        dev_err(dev, "failed to get pwm0: %ld\n", PTR_ERR(ln->pwm0));
        return PTR_ERR(ln->pwm0);
    }

    ln->pwm1 = devm_pwm_get(dev, "pwm1");
    if (IS_ERR(ln->pwm1)) {
        dev_err(dev, "failed to get pwm1: %ld\n", PTR_ERR(ln->pwm1));
        return PTR_ERR(ln->pwm1);
    }

    /* 從設備樹讀取PWM參數 */
    if (of_property_read_u32(dev->of_node, "period-ns", &ln->period_ns)) {
        ln->period_ns = 200000; 
        dev_info(dev, "使用預設週期: %u ns\n", ln->period_ns);
    }

    if (of_property_read_u32(dev->of_node, "duty-ns", &ln->duty_ns)) {
        ln->duty_ns = 100000; // 預設1.5ms
        dev_info(dev, "使用預設占空比: %u ns\n", ln->duty_ns);
    }

    dev_info(dev, "Motor initialized: period=%u ns, duty=%u ns\n", 
             ln->period_ns, ln->duty_ns);

    /* 初始化PWM配置 */
    ret = pwm_config(ln->pwm0, 0, ln->period_ns);
    if (ret < 0) {
        dev_err(dev, "PWM0配置失敗: %d\n", ret);
        return ret;
    }

    ret = pwm_config(ln->pwm1, 0, ln->period_ns);
    if (ret < 0) {
        dev_err(dev, "PWM1配置失敗: %d\n", ret);
        return ret;
    }

    /* 啟用PWM */
    ret = pwm_enable(ln->pwm0);
    if (ret < 0) {
        dev_err(dev, "PWM0啟用失敗: %d\n", ret);
        return ret;
    }

    ret = pwm_enable(ln->pwm1);
    if (ret < 0) {
        dev_err(dev, "PWM1啟用失敗: %d\n", ret);
        pwm_disable(ln->pwm0);
        return ret;
    }

    ln->pwm_configured = true;
    dev_info(dev, "Motor driver probe completed successfully\n");

    return 0;
}

static void motor_remove(struct platform_device *pdev)
{
    struct l298n_dev *ln = platform_get_drvdata(pdev);

    dev_info(&pdev->dev, "Motor driver remove started\n");

    if (!ln)
        pr_info("err\n");
        //REMOVE_RETURN_VALUE;

    if (ln->pwm_configured) {
        /* 停止PWM並設定安全狀態 */
        pwm_config(ln->pwm0, 0, ln->period_ns);
        pwm_config(ln->pwm1, 0, ln->period_ns);
        
        pwm_disable(ln->pwm0);
        pwm_disable(ln->pwm1);
        
        ln->pwm_configured = false;
    }

    global_motor_dev = NULL;
    dev_info(&pdev->dev, "Motor driver remove completed\n");
    
    // REMOVE_RETURN_VALUE;
}

static const struct of_device_id motor_of_match[] = {
    { .compatible = "motor" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, motor_of_match);

static struct platform_driver motor_platform_driver = {
    .driver = {
        .name = "motor",
        .of_match_table = motor_of_match,
    },
    .probe = motor_probe,
    .remove = motor_remove,
};

/* ===== GPIO 控制函數 ===== */

static void gpio_set_function(int gpio, int func)
{
    u32 reg_offset, shift, val;
    
    if (!global_motor_dev || !global_motor_dev->gpio_base)
        return;
    
    reg_offset = (gpio / 10) * 4;
    shift = (gpio % 10) * 3;
    
    val = readl(global_motor_dev->gpio_base + reg_offset);
    val &= ~(7 << shift);
    val |= (func << shift);
    writel(val, global_motor_dev->gpio_base + reg_offset);
    
    dev_info(&global_motor_dev->pdev->dev, "GPIO%d 設定為功能 %d\n", gpio, func);
}

static void gpio_set_high(int gpio)
{
    if (!global_motor_dev || !global_motor_dev->gpio_base)
        return;
        
    writel(1 << gpio, global_motor_dev->gpio_base + GPSET0);
}

static void gpio_set_low(int gpio)
{
    if (!global_motor_dev || !global_motor_dev->gpio_base)
        return;
        
    writel(1 << gpio, global_motor_dev->gpio_base + GPCLR0);
}

static int setup_gpio_pins(void)
{
    int ret;
    
    if (!global_motor_dev) {
        pr_err("Global motor device not initialized\n");
        return -ENODEV;
    }
    
    dev_info(&global_motor_dev->pdev->dev, "開始設定GPIO腳位\n");


    /* 取得 GPIO descriptors */
    global_motor_dev->gpio_in1 = devm_gpiod_get(&global_motor_dev->pdev->dev, "in1", GPIOD_OUT_LOW);
    if (IS_ERR(global_motor_dev->gpio_in1)) {
            dev_err(&global_motor_dev->pdev->dev, "無法取得 in1 GPIO: %ld\n", PTR_ERR(global_motor_dev->gpio_in1));
            return PTR_ERR(global_motor_dev->gpio_in1);
    }

    global_motor_dev->gpio_in2 = devm_gpiod_get(&global_motor_dev->pdev->dev, "in2", GPIOD_OUT_LOW);
    if (IS_ERR(global_motor_dev->gpio_in2)) {
            dev_err(&global_motor_dev->pdev->dev, "無法取得 in2 GPIO: %ld\n", PTR_ERR(global_motor_dev->gpio_in2));
            return PTR_ERR(global_motor_dev->gpio_in2);
    }

    global_motor_dev->gpio_in3 = devm_gpiod_get(&global_motor_dev->pdev->dev, "in3", GPIOD_OUT_LOW);
    if (IS_ERR(global_motor_dev->gpio_in3)) {
            dev_err(&global_motor_dev->pdev->dev, "無法取得 in3 GPIO: %ld\n", PTR_ERR(global_motor_dev->gpio_in3));
            return PTR_ERR(global_motor_dev->gpio_in3);
    }

    global_motor_dev->gpio_in4 = devm_gpiod_get(&global_motor_dev->pdev->dev, "in4", GPIOD_OUT_LOW);
    if (IS_ERR(global_motor_dev->gpio_in4)) {
            dev_err(&global_motor_dev->pdev->dev, "無法取得 in4 GPIO: %ld\n", PTR_ERR(global_motor_dev->gpio_in4));
            return PTR_ERR(global_motor_dev->gpio_in4);
    }

    global_motor_dev->gpio_requested = true;

    desc_to_gpio(global_motor_dev->gpio_in1),
    desc_to_gpio(global_motor_dev->gpio_in2),
    desc_to_gpio(global_motor_dev->gpio_in3),
    desc_to_gpio(global_motor_dev->gpio_in4),
        
    
    /* 如果有映射GPIO記憶體，設定PWM功能 */
    
        gpio_set_function(PWM_LEFT_GPIO, 2);    // GPIO18 PWM0_0
        gpio_set_function(PWM_RIGHT_GPIO, 2);   // GPIO19 PWM0_1
    
    
    dev_info(&global_motor_dev->pdev->dev, "GPIO腳位設定完成\n");

    return 0;
}

/* ===== PWM 控制函數 ===== */

static int set_motor_speed_pwm(struct pwm_device *pwm, int speed)
{
    unsigned int duty;
    
    if (!pwm || IS_ERR(pwm) || !global_motor_dev)
        return -ENODEV;

    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    
    duty = (global_motor_dev->period_ns * speed) / 100;

    printk("284 duty is %d , period is %d\n",global_motor_dev->duty_ns,global_motor_dev->period_ns);
    return pwm_config(pwm, duty, global_motor_dev->period_ns);
}

/* ===== 馬達控制函數 ===== */

static void move_forward(void)
{
    if (!global_motor_dev) return;

    gpiod_set_value(global_motor_dev->gpio_in1, 1);
    gpiod_set_value(global_motor_dev->gpio_in2, 0);
    gpiod_set_value(global_motor_dev->gpio_in3, 1);
    gpiod_set_value(global_motor_dev->gpio_in4, 0);
    pwm_enable(global_motor_dev->pwm0);
    pwm_enable(global_motor_dev->pwm1);

    if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm0, 80);
    }
    if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm1, 80);
    }
    dev_info(&global_motor_dev->pdev->dev, "直線前進\n");
}

static void move_backward(void)
{
    if (!global_motor_dev) return;

    gpiod_set_value(global_motor_dev->gpio_in1, 0);
    gpiod_set_value(global_motor_dev->gpio_in2, 1);
    gpiod_set_value(global_motor_dev->gpio_in3, 0);
    gpiod_set_value(global_motor_dev->gpio_in4, 1);
    pwm_enable(global_motor_dev->pwm0);
    pwm_enable(global_motor_dev->pwm1);

    if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm0, 80);
    }
    if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm1, 80);
    }
    dev_info(&global_motor_dev->pdev->dev, "直線後退\n");
}

static void left_motor_forward(void)
{
    if (!global_motor_dev) return;
    
    gpiod_set_value(global_motor_dev->gpio_in3, 1);
    gpiod_set_value(global_motor_dev->gpio_in4, 0);
    pwm_enable(global_motor_dev->pwm1);
    if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm1, 80);
    }
    
    dev_info(&global_motor_dev->pdev->dev, "左馬達設定為前進\n");
}

static void left_motor_backward(void)
{
    if (!global_motor_dev) return;
    
    gpiod_set_value(global_motor_dev->gpio_in3, 0);
    gpiod_set_value(global_motor_dev->gpio_in4, 1);
    pwm_enable(global_motor_dev->pwm1);
    dev_info(&global_motor_dev->pdev->dev, "左馬達設定為後退\n");
}

static void left_motor_stop(void)
{
    if (!global_motor_dev) return;
    
    gpiod_set_value(global_motor_dev->gpio_in3, 0);
    gpiod_set_value(global_motor_dev->gpio_in4, 0);
    pwm_enable(global_motor_dev->pwm1);
    global_motor_dev->left_speed = 0;
    
    if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm1, 0);
    }
    
    dev_info(&global_motor_dev->pdev->dev, "左馬達停止\n");
}

static void right_motor_forward(void)
{
    if (!global_motor_dev) return;
    
    gpiod_set_value(global_motor_dev->gpio_in1, 1);
    gpiod_set_value(global_motor_dev->gpio_in2, 0);
    pwm_enable(global_motor_dev->pwm0);
    
    if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm0, 80);
    }
    
    dev_info(&global_motor_dev->pdev->dev, "右馬達設定為前進\n");
}

static void right_motor_backward(void)
{
    if (!global_motor_dev) return;
    
    gpiod_set_value(global_motor_dev->gpio_in1, 0);
    gpiod_set_value(global_motor_dev->gpio_in2, 1);
    pwm_enable(global_motor_dev->pwm0);
    dev_info(&global_motor_dev->pdev->dev, "右馬達設定為後退\n");
}

static void right_motor_stop(void)
{
    if (!global_motor_dev) return;

    gpiod_set_value(global_motor_dev->gpio_in1, 0);
    gpiod_set_value(global_motor_dev->gpio_in2, 0);
    pwm_enable(global_motor_dev->pwm0);
    global_motor_dev->right_speed = 0;
    
    if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
        set_motor_speed_pwm(global_motor_dev->pwm0, 0);
    }
    
    dev_info(&global_motor_dev->pdev->dev, "右馬達停止\n");
}

static void stop_all_motors(void)
{
    /*left_motor_stop();
    right_motor_stop();*/
    if (global_motor_dev->pwm0){
        pwm_disable(global_motor_dev->pwm0);
        gpiod_set_value(global_motor_dev->gpio_in1, 0);
        gpiod_set_value(global_motor_dev->gpio_in2, 0);   
    }
    if (global_motor_dev->pwm1){
        pwm_disable(global_motor_dev->pwm1);
        gpiod_set_value(global_motor_dev->gpio_in3, 0);
        gpiod_set_value(global_motor_dev->gpio_in4, 0);
    }
  dev_info(&global_motor_dev->pdev->dev, "所有馬達已停止\n");      
}

/* ===== 字符設備操作函數 ===== */

static int motor_open(struct inode *inode, struct file *file)
{
    if (global_motor_dev)
        dev_info(&global_motor_dev->pdev->dev, "馬達設備已開啟\n");
    return 0;
}

static int motor_release(struct inode *inode, struct file *file)
{
    if (global_motor_dev)
        dev_info(&global_motor_dev->pdev->dev, "馬達設備已關閉，停止所有馬達\n");
    stop_all_motors();
    return 0;
}

static long motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int speed;
    unsigned int duty_ns;
    unsigned int new_period;

    
    if (!global_motor_dev) {
        pr_err("Motor device not initialized\n");
        return -ENODEV;
    }
    
    if (_IOC_TYPE(cmd) != MOTOR_IOC_MAGIC) {
        dev_err(&global_motor_dev->pdev->dev, "無效的ioctl magic number\n");
        return -ENOTTY;
    }
    
    dev_info(&global_motor_dev->pdev->dev, "收到ioctl指令: %u, 參數: %lu\n", cmd, arg);
    
    switch (cmd) {
        case IOCTL_SET_PERIOD:
        
        if (copy_from_user(&new_period, (unsigned int __user *)arg, sizeof(new_period)))
            return -EFAULT;

        global_motor_dev->period_ns = new_period;

        /* 更新 PWM */
        if (global_motor_dev->pwm0)
            pwm_config(global_motor_dev->pwm0, (global_motor_dev->left_speed * new_period)/100, new_period);
        if (global_motor_dev->pwm1)
            pwm_config(global_motor_dev->pwm1, (global_motor_dev->right_speed * new_period)/100, new_period);

        dev_info(&global_motor_dev->pdev->dev, "PWM週期更新為 %u ns\n", new_period);
        break;
        

        case IOCTL_SET_SPEED_LEFT:
        
            printk("IOCTL_SET_SPEED_LEFT\n");
            if (copy_from_user(&duty_ns, (unsigned int __user *)arg, sizeof(duty_ns)))
                return -EFAULT;

            if (duty_ns > global_motor_dev->period_ns) {
                dev_err(&global_motor_dev->pdev->dev, "占空比超過週期: %u > %u\n",
                duty_ns, global_motor_dev->period_ns);
                return -EINVAL;
            }

            // 計算百分比
            int speed = (duty_ns * 100) / global_motor_dev->period_ns;
            global_motor_dev->left_speed = speed;

            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = set_motor_speed_pwm(global_motor_dev->pwm1, speed);
                if(ret == 0)
                {
                    //gpiod_set_value(global_motor_dev->gpio_in3, 0);
                    //gpiod_set_value(global_motor_dev->gpio_in4, 1);
                    pwm_enable(global_motor_dev->pwm1);
                } 
            }

            dev_info(&global_motor_dev->pdev->dev, "左馬達速度設定為: %d%%\n", speed);
            break;
        
            
        case IOCTL_SET_SPEED_RIGHT:
        
            if (copy_from_user(&duty_ns, (unsigned int __user *)arg, sizeof(duty_ns)))
                return -EFAULT;

            if (duty_ns > global_motor_dev->period_ns) {
                dev_err(&global_motor_dev->pdev->dev, "占空比超過週期: %u > %u\n",
                duty_ns, global_motor_dev->period_ns);
                return -EINVAL;
            }

            // 計算百分比
            speed = (duty_ns * 100) / global_motor_dev->period_ns;
            global_motor_dev->right_speed = speed;

            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = set_motor_speed_pwm(global_motor_dev->pwm0, speed);
                printk("ret is %d\n",ret);
                if(ret == 0)
                {
                   // gpiod_set_value(global_motor_dev->gpio_in1, 1);
                   // gpiod_set_value(global_motor_dev->gpio_in2, 0);
                    pwm_enable(global_motor_dev->pwm0);
                }
             }

            dev_info(&global_motor_dev->pdev->dev, "右馬達速度設定為: %d%%\n", speed);
            break;
        
        case IOCTL_LEFT_FORWARD:

            gpiod_set_value(global_motor_dev->gpio_in3, 1);
            gpiod_set_value(global_motor_dev->gpio_in4, 0);
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                    ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "左馬達前進\n");
            break;
            
        case IOCTL_LEFT_BACKWARD:

            gpiod_set_value(global_motor_dev->gpio_in3, 0);
            gpiod_set_value(global_motor_dev->gpio_in4, 1);
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "左馬達後退\n");
            break;
            
        case IOCTL_LEFT_STOP:
        
            gpiod_set_value(global_motor_dev->gpio_in3, 0);
            gpiod_set_value(global_motor_dev->gpio_in4, 0);
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                pwm_config(global_motor_dev->pwm1, 0, global_motor_dev->period_ns);
                pwm_enable(global_motor_dev->pwm1);
            }
            dev_info(&global_motor_dev->pdev->dev, "左馬達停止\n");
            break;
        

        case IOCTL_RIGHT_FORWARD:
        
            gpiod_set_value(global_motor_dev->gpio_in1, 1);
            gpiod_set_value(global_motor_dev->gpio_in2, 0);
            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
               ret = pwm_config(global_motor_dev->pwm0, 
                   (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                     global_motor_dev->period_ns);
                    ret = pwm_config(global_motor_dev->pwm0,  (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                    printk("duty cycle %d period %d\n", (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100,global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                    printk("587\n");
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "右馬達前進\n");
            break;
        
        case IOCTL_RIGHT_BACKWARD:

            gpiod_set_value(global_motor_dev->gpio_in1, 0);
            gpiod_set_value(global_motor_dev->gpio_in2, 1);
            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm0, 
                    (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "右馬達後退\n");
            break;
            
        case IOCTL_RIGHT_STOP:

            gpiod_set_value(global_motor_dev->gpio_in1, 0);
            gpiod_set_value(global_motor_dev->gpio_in2, 0);
            if (global_motor_dev->pwm0) {
                pwm_config(global_motor_dev->pwm0, 0, global_motor_dev->period_ns);
                pwm_disable(global_motor_dev->pwm0);
            }
            dev_info(&global_motor_dev->pdev->dev, "右馬達停止\n");
            break;
            
        case IOCTL_BOTH_FORWARD:

            gpiod_set_value(global_motor_dev->gpio_in1, 1);
            gpiod_set_value(global_motor_dev->gpio_in2, 0);
            gpiod_set_value(global_motor_dev->gpio_in3, 1);
            gpiod_set_value(global_motor_dev->gpio_in4, 0);

            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm0, 
                    (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                    if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                }
            }

            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                    if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "直線前進\n");
            break;
            
        case IOCTL_BOTH_BACKWARD:

            gpiod_set_value(global_motor_dev->gpio_in1, 0);
            gpiod_set_value(global_motor_dev->gpio_in2, 1);
            gpiod_set_value(global_motor_dev->gpio_in3, 0);
            gpiod_set_value(global_motor_dev->gpio_in4, 1);
            
            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm0, 
                    (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                }
            }
            
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "直線後退\n");
            break;
            
        case IOCTL_BOTH_STOP:

            gpiod_set_value(global_motor_dev->gpio_in1, 0);
            gpiod_set_value(global_motor_dev->gpio_in2, 0);
            gpiod_set_value(global_motor_dev->gpio_in3, 0);
            gpiod_set_value(global_motor_dev->gpio_in4, 0);
            if (global_motor_dev->pwm0) {
                pwm_config(global_motor_dev->pwm0, 0, global_motor_dev->period_ns);
                pwm_disable(global_motor_dev->pwm0);
            }
            if (global_motor_dev->pwm1) {
                pwm_config(global_motor_dev->pwm1, 0, global_motor_dev->period_ns);
                pwm_disable(global_motor_dev->pwm1);
            }
            dev_info(&global_motor_dev->pdev->dev, "煞車停止\n");
            break;
        
        case IOCTL_TURN_LEFT:

            gpiod_set_value(global_motor_dev->gpio_in1, 1);
            gpiod_set_value(global_motor_dev->gpio_in2, 0);
            gpiod_set_value(global_motor_dev->gpio_in3, 0);
            gpiod_set_value(global_motor_dev->gpio_in4, 1);
            
            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm0, 
                    (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                }
            }
            
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            }
            dev_info(&global_motor_dev->pdev->dev, "執行左轉\n");
            break;
            
        case IOCTL_TURN_RIGHT:

           gpiod_set_value(global_motor_dev->gpio_in1, 0); 
            gpiod_set_value(global_motor_dev->gpio_in2, 1);
            gpiod_set_value(global_motor_dev->gpio_in3, 1); 
            gpiod_set_value(global_motor_dev->gpio_in4, 0);
            
            if (global_motor_dev->pwm0 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm0, 
                    (global_motor_dev->right_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                    if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm0);
                }
            }
            
            if (global_motor_dev->pwm1 && global_motor_dev->pwm_configured) {
                ret = pwm_config(global_motor_dev->pwm1, 
                    (global_motor_dev->left_speed * global_motor_dev->period_ns) / 100, 
                    global_motor_dev->period_ns);
                    if (ret == 0) {
                    pwm_enable(global_motor_dev->pwm1);
                }
            } 
            dev_info(&global_motor_dev->pdev->dev, "執行右轉\n");
            break;
            
        default:
            dev_err(&global_motor_dev->pdev->dev, "未知的ioctl指令: %u\n", cmd);
            return -EINVAL;
    }
    
    return ret;
}

static const struct file_operations motor_fops = {
    .owner = THIS_MODULE,
    .open = motor_open,
    .release = motor_release,
    .unlocked_ioctl = motor_ioctl,
};

/* ===== 字符設備初始化 ===== */

static int create_char_device(void)
{
    int ret;
    
    if (!global_motor_dev) {
        pr_err("Global motor device not initialized\n");
        return -ENODEV;
    }
    
    /* 分配字符設備號 */
    ret = alloc_chrdev_region(&global_motor_dev->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&global_motor_dev->pdev->dev, "字符設備號分配失敗: %d\n", ret);
        return ret;
    }
    
    /* 初始化並註冊字符設備 */
    cdev_init(&global_motor_dev->cdev, &motor_fops);
    global_motor_dev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&global_motor_dev->cdev, global_motor_dev->dev_num, 1);
    if (ret < 0) {
        dev_err(&global_motor_dev->pdev->dev, "字符設備註冊失敗: %d\n", ret);
        unregister_chrdev_region(global_motor_dev->dev_num, 1);
        return ret;
    }

    /* 創建設備類別 */
    global_motor_dev->class = class_create(CLASS_NAME);
    if (IS_ERR(global_motor_dev->class)) {
        dev_err(&global_motor_dev->pdev->dev, "設備類別創建失敗: %ld\n", 
                PTR_ERR(global_motor_dev->class));
        ret = PTR_ERR(global_motor_dev->class);
        cdev_del(&global_motor_dev->cdev);
        unregister_chrdev_region(global_motor_dev->dev_num, 1);
        return ret;
    }

    /* 創建設備節點 */
    global_motor_dev->device = device_create(global_motor_dev->class, NULL, 
                                            global_motor_dev->dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(global_motor_dev->device)) {
        dev_err(&global_motor_dev->pdev->dev, "設備節點創建失敗: %ld\n", 
                PTR_ERR(global_motor_dev->device));
        ret = PTR_ERR(global_motor_dev->device);
        class_destroy(global_motor_dev->class);
        cdev_del(&global_motor_dev->cdev);
        unregister_chrdev_region(global_motor_dev->dev_num, 1);
        return ret;
    }

    dev_info(&global_motor_dev->pdev->dev, "字符設備創建成功: /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void destroy_char_device(void)
{
    if (!global_motor_dev)
        return;
        
    if (!IS_ERR_OR_NULL(global_motor_dev->device))
        device_destroy(global_motor_dev->class, global_motor_dev->dev_num);
    if (!IS_ERR_OR_NULL(global_motor_dev->class))
        class_destroy(global_motor_dev->class);
    
    cdev_del(&global_motor_dev->cdev);
    unregister_chrdev_region(global_motor_dev->dev_num, 1);
}

/* ===== 模組初始化和清理函數 ===== */

static int __init motor_init(void)
{
    int ret;

    pr_info("L298N馬達驅動程式開始載入\n");
    
    /* 註冊平台驅動程式 */
    ret = platform_driver_register(&motor_platform_driver);
    if (ret < 0) {
        pr_err("平台驅動程式註冊失敗: %d\n", ret);
        return ret;
    }
    
    /* 等待平台設備探測完成 */
    if (!global_motor_dev) {
        pr_warn("等待設備樹匹配...\n");
        /* 在實際應用中，如果沒有匹配的設備樹節點，probe不會被呼叫 */
        platform_driver_unregister(&motor_platform_driver);
        return -ENODEV;
    }
    
    /* 映射GPIO記憶體 */
    global_motor_dev->gpio_base = ioremap(BCM2711_GPIO_BASE, GPIO_SIZE);
    if (!global_motor_dev->gpio_base) {
        dev_warn(&global_motor_dev->pdev->dev, "GPIO記憶體映射失敗，將使用標準GPIO API\n");
    }
    
    /* 設定GPIO腳位 */
    ret = setup_gpio_pins();
    if (ret < 0) {
        dev_err(&global_motor_dev->pdev->dev, "GPIO腳位設定失敗: %d\n", ret);
        goto err_gpio;
    }
    
    /* 創建字符設備 */
    ret = create_char_device();
    if (ret < 0) {
        dev_err(&global_motor_dev->pdev->dev, "字符設備創建失敗: %d\n", ret);
        goto err_gpio;
    }
    
    pr_info("L298N馬達驅動程式載入成功!\n");
    pr_info("設備節點: /dev/%s\n", DEVICE_NAME);
    pr_info("GPIO配置: IN1=%d, IN2=%d, IN3=%d, IN4=%d\n", 
           MOTOR_IN1_GPIO, MOTOR_IN2_GPIO, MOTOR_IN3_GPIO, MOTOR_IN4_GPIO);
    pr_info("PWM配置: LEFT=GPIO%d, RIGHT=GPIO%d\n", PWM_LEFT_GPIO, PWM_RIGHT_GPIO);

    return 0;

err_gpio:
    if (global_motor_dev && global_motor_dev->gpio_base)
        iounmap(global_motor_dev->gpio_base);
    platform_driver_unregister(&motor_platform_driver);
    
    return ret;
}

static void __exit motor_exit(void)
{
    pr_info("開始卸載L298N馬達驅動程式\n");
    
    /* 停止所有馬達 */
    stop_all_motors();
    
    /* 清理字符設備 */
    destroy_char_device();
    
    /* 清理GPIO記憶體映射 */
    if (global_motor_dev && global_motor_dev->gpio_base)
        iounmap(global_motor_dev->gpio_base);
    
    /* 註銷平台驅動程式 */
    platform_driver_unregister(&motor_platform_driver);
    
    pr_info("L298N馬達驅動程式卸載完成\n");
}

module_init(motor_init);
module_exit(motor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("L298N Motor Control Driver with Device Tree Support");
MODULE_VERSION("2.0");