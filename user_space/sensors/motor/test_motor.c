#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include "motor_gpio.h"


#define DEVICE_PATH "/dev/motor0"

/* 全域變數 */
static int motor_fd = -1;
static struct termios orig_termios;

/* 馬達狀態結構 */
typedef struct {
    int left_speed;     // 左馬達速度 (0-100%)
    int right_speed;    // 右馬達速度 (0-100%)
    int left_dir;       // 左馬達方向 (1:前進, -1:後退, 0:停止)
    int right_dir;      // 右馬達方向 (1:前進, -1:後退, 0:停止)
    unsigned int period_ns; // PWM週期
} motor_state_t;

static motor_state_t motor_state = {
    .left_speed = 40,
    .right_speed = 40,
    .left_dir = 0,
    .right_dir = 0,
    .period_ns = 200000  // 20ms = 50Hz
};

/* ===== 終端控制函數 ===== */

/* 設定終端為非緩衝模式 */
void set_raw_mode(void) {
    struct termios raw;
    
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);  // 關閉回顯和緩衝
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* 恢復終端設定 */
void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* 訊號處理函數 - 程式結束時清理 */
void cleanup_and_exit(int sig) {
    printf("\n正在停止所有馬達...\n");
    if (motor_fd >= 0) {
        ioctl(motor_fd, IOCTL_BOTH_STOP);
        close(motor_fd);
    }
    restore_terminal();
    printf("程式已結束\n");
    exit(0);
}

/* ===== 馬達控制函數 ===== */

/* 開啟馬達設備 */
int open_motor_device(void) {
    motor_fd = open(DEVICE_PATH, O_RDWR);
    if (motor_fd < 0) {
        perror("無法開啟馬達設備");
        printf("請確認:\n");
        printf("1. 驅動程式已載入: lsmod | grep motor\n");
        printf("2. 設備節點存在: ls -l /dev/motor\n");
        printf("3. 權限正確: sudo chmod 666 /dev/motor\n");
        return -1;
    }
    return 0;
}

/* 設定PWM週期 */
int set_pwm_period(unsigned int period_ns) {
    if (ioctl(motor_fd, IOCTL_SET_PERIOD, &period_ns) < 0) {
        perror("設定PWM週期失敗");
        return -1;
    }
    motor_state.period_ns = period_ns;
    printf("PWM週期設定為: %u ns (%.1f Hz)\n", period_ns, 1000000000.0/period_ns);
    return 0;
}

/* 速度百分比轉換為占空比(奈秒) */
unsigned int speed_to_duty_ns(int speed_percent) {
    if (speed_percent < 0) speed_percent = 0;
    if (speed_percent > 100) speed_percent = 100;
    
    // 將0-100%映射到0-週期時間的占空比
    return (motor_state.period_ns * speed_percent) / 100;
}

/* 設定左馬達速度和方向 */
int set_left_motor(int speed, int direction) {
    unsigned int duty_ns;
    
    motor_state.left_speed = speed;
    motor_state.left_dir = direction;
    
    // 設定速度(PWM占空比)
    duty_ns = speed_to_duty_ns(speed);
    if (ioctl(motor_fd, IOCTL_SET_SPEED_LEFT, &duty_ns) < 0) {
        perror("設定左馬達速度失敗");
        return -1;
    }
    
    // 設定方向
    if (direction > 0) {
        if (ioctl(motor_fd, IOCTL_LEFT_FORWARD) < 0) {
            perror("左馬達前進指令失敗");
            return -1;
        }
    } else if (direction < 0) {
        if (ioctl(motor_fd, IOCTL_LEFT_BACKWARD) < 0) {
            perror("左馬達後退指令失敗");
            return -1;
        }
    }else{
        if (ioctl(motor_fd, IOCTL_LEFT_STOP) < 0) {
        perror("左馬達停止指令失敗");
        return -1;
        }
    }
    return 0;
    
}
/* 設定右馬達速度和方向 */
int set_right_motor(int speed, int direction) {
    unsigned int duty_ns;
    
    motor_state.right_speed = speed;
    motor_state.right_dir = direction;
    
    // 設定速度(PWM占空比)  
    duty_ns = speed_to_duty_ns(speed);
    if (ioctl(motor_fd, IOCTL_SET_SPEED_RIGHT, &duty_ns) < 0) {
        perror("設定右馬達速度失敗");
        return -1;
    }
    
    // 設定方向
    if (direction > 0) {
        if (ioctl(motor_fd, IOCTL_RIGHT_FORWARD) < 0) {
            perror("右馬達前進指令失敗");
            return -1;
        }
    } else if (direction < 0) {
        if (ioctl(motor_fd, IOCTL_RIGHT_BACKWARD) < 0) {
            perror("右馬達後退指令失敗");
            return -1;
        }
    }else{
        if (ioctl(motor_fd, IOCTL_RIGHT_STOP) < 0) {
        perror("右馬達停止指令失敗");
        return -1;
        }
    return 0;
    }
}
/* 停止所有馬達 */
int stop_all_motors(void) {
    if (ioctl(motor_fd, IOCTL_BOTH_STOP) < 0) {
        perror("停止馬達失敗");
        return -1;
    }
    motor_state.left_dir = 0;
    motor_state.right_dir = 0;
    printf("所有馬達已停止\n");
    return 0;
}

/* ===== 預定義動作函數 ===== */

/* 前進 */
void move_forward(void) {
    set_left_motor(motor_state.left_speed, 1);
    set_right_motor(motor_state.right_speed, 1);
    printf("前進 - 左馬達:%d%%, 右馬達:%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* 後退 */
void move_backward(void) {
    set_left_motor(motor_state.left_speed, -1);
    set_right_motor(motor_state.right_speed, -1);
    printf("後退 - 左馬達:%d%%, 右馬達:%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* 左轉 */
void turn_left(void) {
    set_left_motor(motor_state.left_speed, -1);  // 左馬達反轉
    set_right_motor(motor_state.right_speed, 1); // 右馬達前進
    printf("左轉 - 左馬達:-%d%%, 右馬達:+%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* 右轉 */
void turn_right(void) {
    set_left_motor(motor_state.left_speed, 1);   // 左馬達前進
    set_right_motor(motor_state.right_speed, -1); // 右馬達反轉
    printf("右轉 - 左馬達:+%d%%, 右馬達:-%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* 遞增 */
void add_motor_speed(void) {
    set_left_motor(motor_state.left_speed, motor_state.left_dir);   // 左馬達加速
    set_right_motor(motor_state.right_speed, motor_state.right_dir); // 左馬達加速
    printf("加速 - 左馬達:+%d%%, 右馬達:+%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* 遞減 */
void reduce_motor_speed(void) {
    set_left_motor(motor_state.left_speed, motor_state.left_dir);   // 左馬達加速
    set_right_motor(motor_state.right_speed, motor_state.right_dir); // 左馬達加速
    printf("加速 - 左馬達:-%d%%, 右馬達:-%d%%\n", 
           motor_state.left_speed, motor_state.right_speed);
}

/* ===== 顯示函數 ===== */

/* 顯示當前狀態 */
void display_status(void) {
    printf("\n=== 馬達狀態 ===\n");
    printf("左馬達: %d%% ", motor_state.left_speed);
    if (motor_state.left_dir > 0) printf("(前進)");
    else if (motor_state.left_dir < 0) printf("(後退)");
    else printf("(停止)");
    
    printf("\n右馬達: %d%% ", motor_state.right_speed);
    if (motor_state.right_dir > 0) printf("(前進)");
    else if (motor_state.right_dir < 0) printf("(後退)");  
    else printf("(停止)");
    
    printf("\nPWM週期: %u ns (%.1f Hz)\n", 
           motor_state.period_ns, 1000000000.0/motor_state.period_ns);
    printf("================\n");
}

/* 顯示幫助資訊 */
void show_help(void) {
    printf("\n=== L298N 馬達控制程式 ===\n");
    printf("控制鍵:\n");
    printf("  W/w - 前進\n");
    printf("  S/s - 後退\n");
    printf("  A/a - 左轉\n");
    printf("  D/d - 右轉\n");
    printf("  空白鍵 - 停止\n");
    printf("\n速度控制:\n");
    printf("  +/= - 增加速度\n");
    printf("  -/_ - 減少速度\n");
    printf("  1-9 - 直接設定速度(10%-90%)\n");
    printf("  0   - 設定速度為100%%\n");
    printf("\n其他:\n");
    printf("  i/I - 顯示狀態\n");
    printf("  h/H - 顯示幫助\n");
    printf("  q/Q - 退出程式\n");
    printf("========================\n");
}

/* ===== 主程式 ===== */

/* 處理鍵盤輸入 */
void handle_input(char key) {
    int new_speed;
    
    switch(key) {
        case 'w':
        case 'W':
            move_forward();
            break;
            
        case 's':
        case 'S':
            move_backward();
            break;
            
        case 'a':
        case 'A':
            turn_left();
            break;
            
        case 'd':
        case 'D':
            turn_right();
            break;
            
        case ' ':  // 空白鍵
            stop_all_motors();
            break;
            
        case '+':
            new_speed = motor_state.left_speed + 10;
            if (new_speed > 100) new_speed = 100;
            motor_state.left_speed = new_speed;
            motor_state.right_speed = new_speed;

            add_motor_speed();   
            printf("速度增加至: %d%%\n", new_speed);
            break;
            
        case '-':
            new_speed = motor_state.left_speed - 10;
            if (new_speed < 0) new_speed = 0;
            motor_state.left_speed = new_speed;
            motor_state.right_speed = new_speed;

            reduce_motor_speed();
            printf("速度減少至: %d%%\n", new_speed);
            break;
            
        /*case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            new_speed = (key - '0') * 10;
            motor_state.left_speed = new_speed;
            motor_state.right_speed = new_speed;
            printf("速度設定為: %d%%\n", new_speed);
            break;*/
            
        case '0':
            motor_state.left_speed = 100;
            motor_state.right_speed = 100;
            printf("速度設定為: 100%%\n");
            break;
            
        case 'i':
        case 'I':
            display_status();
            break;
            
        case 'h':
        case 'H':
            show_help();
            break;
            
        case 'q':
        case 'Q':
            cleanup_and_exit(0);
            break;
            
        default:
            printf("未知指令: '%c' (按 h 查看幫助)\n", key);
            break;
    }
}

/* 互動模式主迴圈 */
void interactive_mode(void) {
    char key;
    
    printf("進入互動控制模式 (按 h 查看幫助, q 退出)\n");
    show_help();
    
    while (1) {
        printf("\n請按鍵: ");
        fflush(stdout);
        
        key = getchar();
        if (key == '\n') continue;  // 忽略換行符
        
        handle_input(key);
    }
}

/* 指令列模式 */
void command_mode(int argc, char *argv[]) {
    if (argc < 2) {
        printf("指令格式: %s <指令> [參數]\n", argv[0]);
        printf("指令:\n");
        printf("  forward <speed>     - 前進 (speed: 0-100)\n");
        printf("  backward <speed>    - 後退 (speed: 0-100)\n");
        printf("  left <speed>        - 左轉 (speed: 0-100)\n");
        printf("  right <speed>       - 右轉 (speed: 0-100)\n");
        printf("  stop                - 停止\n");
        printf("  status              - 顯示狀態\n");
        printf("  interactive         - 進入互動模式\n");
        return;
    }
    
    if (strcmp(argv[1], "forward") == 0) {
        if (argc > 2) {
            int speed = atoi(argv[2]);
            motor_state.left_speed = speed;
            motor_state.right_speed = speed;
        }
        move_forward();
    }
    else if (strcmp(argv[1], "backward") == 0) {
        if (argc > 2) {
            int speed = atoi(argv[2]);
            motor_state.left_speed = speed;
            motor_state.right_speed = speed;
        }
        move_backward();
    }
    else if (strcmp(argv[1], "left") == 0) {
        if (argc > 2) {
            int speed = atoi(argv[2]);
            motor_state.left_speed = speed;
            motor_state.right_speed = speed;
        }
        turn_left();
    }
    else if (strcmp(argv[1], "right") == 0) {
        if (argc > 2) {
            int speed = atoi(argv[2]);
            motor_state.left_speed = speed;
            motor_state.right_speed = speed;
        }
        turn_right();
    }
    else if (strcmp(argv[1], "stop") == 0) {
        stop_all_motors();
    }
    else if (strcmp(argv[1], "status") == 0) {
        display_status();
    }
    else if (strcmp(argv[1], "interactive") == 0) {
        set_raw_mode();
        interactive_mode();
    }
    else {
        printf("未知指令: %s\n", argv[1]);
    }
}

 int main(int argc, char *argv[]) {
    printf("L298N 馬達控制程式啟動中...\n");
    
    /* 設定訊號處理 */
    signal(SIGINT, cleanup_and_exit);   // Ctrl+C
    signal(SIGTERM, cleanup_and_exit);  // 終止訊號
    
    /* 開啟馬達設備 */
    if (open_motor_device() < 0) {
        return 1;
    }
    
    printf("馬達設備開啟成功\n");
    
    /* 設定預設PWM週期 */
    set_pwm_period(motor_state.period_ns);
    
    /* 根據參數決定執行模式 */
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "interactive") == 0)) {
        /* 無參數或指定interactive - 進入互動模式 */
        set_raw_mode();
        interactive_mode();
    } else {
        /* 有參數 - 指令模式 */
        command_mode(argc, argv);
    }
    
    /* 清理並結束 */
     cleanup_and_exit(0);
     return 0;
 }
