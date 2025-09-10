// Car Sensors GPIO Definitions


// UART ( Rpi4 <-> Pico w )
// GPIO 14 TXD
// GPIO 15 RXD


// TRCT5000 循跡感測器 
#define TCRT5000_LEFT 16
#define GPIO16_FSEL  GPFSEL1
#define GPIO16_BIT_SHIFT 18 
	
#define TCRT5000_RIGHT 17
#define GPIO17_FSEL  GPFSEL1
#define GPIO17_BIT_SHIFT 21

#define TCRT5000_MIDDLE 18
#define GPIO18_FSEL  GPFSEL1
#define GPIO18_BIT_SHIFT 24


// L298N 馬達控制器


// SRF05 超音波測距


// LED 狀態燈
//#define GREEN 		// 綠燈(車輛正常行駛)
//#define YELLOW 		// 黃燈(異常狀況)
//#define RED 			// 紅燈(車輛停止)
//#define ORANGE_LEFT 		// 橘燈(左邊方向燈)
//#define ORANGE_RIGHT 		// 橘燈(右邊方向燈)
//#define WHITE 		// 白燈(倒車燈)


