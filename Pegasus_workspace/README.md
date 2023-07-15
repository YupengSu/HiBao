# HiBao: Pegasus_Work
> 将每一个 demo 放入 Hi3861 SDK 的 app 文件夹中， 改变 BUILD.gn 编译不同功能。 

## dht11_demo: 温湿度传感器
* 启用引脚：GPIO11/UART2_TXD/SPI0_RXD/ADC5/PWM2_OUT
* 板端接线：使用机器人拓展版传感器1
* 实现功能：定时接收当前温度与湿度

## infrared_demo: 红外检测传感器
* 启用引脚：GPIO12/UART2_RXD/SPI0_CS1/ADC0/PWM3_OUT
* 板端接线：使用机器人拓展版传感器2
* 实现功能：定时接收模块前有无障碍物