#ifndef __BUZZER_H__
#define __BUZZER_H__

#define BUZZER_GPIO 6   // 預設 GPIO 腳位

// API 函式
int buzzer_open(void);     // 開啟蜂鳴器
int buzzer_write(int on);  // 寫入(on=1開, 0關 )
int buzzer_close(void);    // 關閉蜂鳴器

#endif
