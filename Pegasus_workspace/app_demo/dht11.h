#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

// 等待DHT11的回应
// 返回1:未检测到DHT11的存在
// 返回0:存在
uint8_t DHT11_Check(void);

// 复位DHT11
void DHT11_Rst(void);

// 从DHT11读取一个位
// 返回值：1/0
uint8_t DHT11_Read_Bit(void);

// 从DHT11读取一个字节
// 返回值：读到的数据
uint8_t DHT11_Read_Byte(void);

// 从DHT11读取一次数据
// temp: 温度值 (范围: 0~50°)
// humi: 湿度值 (范围: 20%~90%)
// 返回值：0, 正常; 1, 读取失败
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif /* DHT11_H */

