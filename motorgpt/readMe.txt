//dtc -@ -I dts -O dtb -o motor.dtbo motor.dts//    <<--.dts編譯

cp motor.dtbo  /boot/firmware/overlays/             <<----複製

dtoverlay motor                                     <<-----卦載device treee

dtoverlay -l                                        <<-----查看卦載目錄

dtoverlay -r motor                                  <<--------卸載



