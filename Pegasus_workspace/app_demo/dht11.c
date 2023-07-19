/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

#define DH_SENSOR_STACK_SIZE   2048
#define DH_SENSOR_GPIO         11// for hispark_pegasus
#define NUM 1


#define DHT11_GPIO DH_SENSOR_GPIO
									   
#define	DHT11_DQ_OUT_High IoTGpioSetOutputVal(DHT11_GPIO, 1); //设置GPIO输出高电平
#define	DHT11_DQ_OUT_Low IoTGpioSetOutputVal(DHT11_GPIO, 0); //设置GPIO输出低电平  
/****************************************
设置端口为输出
*****************************************/
void DHT11_IO_OUT(void)
{
    //设置GPIO_11为输出模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_OUT);
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void) 	   
{   
	uint8_t retry = 0;
    IotGpioValue DHT11_DQ_IN;
	
    DHT11_IO_IN();//SET INPUT	 

    while (GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN) && retry < 100)//DHT11会拉低40~80us
    {
		retry++;
		hi_udelay(1);
	};

	if(retry >= 100) return (uint8_t)1;
	else retry = 0;
    
    while ((!GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN)) && retry < 100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		hi_udelay(1);
	};
	if(retry>=100) return (uint8_t)1;

	return (uint8_t)0;
}
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在    	 
uint8_t DHT11_Init(void)
{	 
	//初始化GPIO
    IoTGpioInit(DHT11_GPIO);

    //设置GPIO_11的复用功能为普通GPIO
	IoSetFunc(DHT11_GPIO, IOT_IO_FUNC_GPIO_11_GPIO);
    //设置GPIO_11为输出模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_OUT);
	//设置GPIO_11输出高电平
    IoTGpioSetOutputVal(DHT11_GPIO, 1);		
	
    DHT11_Rst();
	
    return DHT11_Check();//等待DHT11的回应
} 

//复位DHT11
void DHT11_Rst(void)	   
{                
    DHT11_IO_OUT(); 	//SET OUTPUT
    DHT11_DQ_OUT_Low; 	//拉低DQ
    hi_udelay(20000);//拉低至少18ms
    DHT11_DQ_OUT_High; 	//DQ=1 
	hi_udelay(35);     	//主机拉高20~40us
}

//获取GPIO输入状态
int GPIOGETINPUT(IotIoName id,IotGpioValue *val)
{
    IoTGpioGetInputVal(id,val);
    return *val;
}

/****************************************
设置端口为输入
*****************************************/
void DHT11_IO_IN(void)
{
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_IN);//配置为输入模式
    IoSetPull(DHT11_GPIO, IOT_IO_PULL_NONE);//配置为浮空输入
}



//从DHT11读取一个位
//返回值：1/0
uint8_t DHT11_Read_Bit(void) 			 
{
 	uint8_t retry=0;
    IotGpioValue DHT11_DQ_IN;

    while(GPIOGETINPUT(DHT11_GPIO, &DHT11_DQ_IN) && retry < 100){//等待变为低电平
        retry++;
        hi_udelay(1);
    }
    retry=0;
    while((!GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN)) && retry < 100){//等待变高电平
        retry++;
        hi_udelay(1);
    }
    hi_udelay(40);//等待40us	//用于判断高低电平，即数据1或0
    if(GPIOGETINPUT(DHT11_GPIO,&DHT11_DQ_IN)) return 1; else return 0;
}
//从DHT11读取一个字节
//返回值：读到的数据
uint8_t DHT11_Read_Byte(void)    
{        
    uint8_t i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }						    
    return dat;
}
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)    
{        
 	uint8_t buf[5]={ 0 };
	uint8_t i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])//数据校验
		{
			*humi=buf[0];
			*temp=buf[2];
		}
	}else return 1;
	return 0;	    
}

static void DhtTask(const int *arg)
{   
    (void)arg;
    printf("DhtTask start\r\n");
    // 配置GPIO引脚号和输出值

    uint8_t temp,humi;

    DHT11_Init();
    while (NUM)
    {   
        DHT11_Read_Data(&temp,&humi);/* code */
        //printf("temp:%d,humi:%d\r\n",temp,humi);
    }
    
    return NULL;
}

/*static void LedExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "DhtTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DH_SENSOR_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)DhtTask, NULL, &attr) == NULL) {
        printf("[LedExample] Failed to create LedTask!\n");
    }
}*/

//SYS_RUN(LedExampleEntry);