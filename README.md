#**TCRT5000循跡感測模組**


##-Kernel space
    說明: 
          1. 硬體(Hardware)=> 硬體層面的(感測器、馬達、各式模組)
          2. 硬體抽象層(HAL)=>  註冊(GPIO/UART/Timmer/PWM)、中斷處理Interrups
          3. 裝置驅動層(Device Driver)=> 感測驅動、馬達驅動 
          4. 核心層(Kernel Core)=> 任務排程、中斷管理、任務間通訊    

##-User Space
    說明: 5. 應用層(Application Layer)=> 控制邏輯、數據處理、指令輸出

