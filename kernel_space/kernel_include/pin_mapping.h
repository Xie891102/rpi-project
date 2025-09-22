// Car Sensors GPIO Definitions


// UART ( Rpi4 <-> Pico w )
// GPIO 14 TXD
// GPIO 15 RXD


// TRCT5000 循跡感測器 
#define TCRT5000_LEFT 9
#define GPIO9_FSEL  GPFSEL0
#define GPIO9_BIT_SHIFT 27
	
#define TCRT5000_RIGHT 10
#define GPIO10_FSEL  GPFSEL1
#define GPIO10_BIT_SHIFT 0

#define TCRT5000_MIDDLE 11
#define GPIO11_FSEL  GPFSEL1
#define GPIO11_BIT_SHIFT 3








