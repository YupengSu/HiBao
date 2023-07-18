//线程参考：thread_demo/thread.c

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

//执行串口
#include "uart_control.h"
#include <string.h>
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"

#define UART_BUFF_SIZE 100
#define U_SLEEP_TIME   100000
#define STACK_SIZE 1024*8
#define OS_DELAY 100
#define OS_DELAYONE 20
char data[] = "0"; // 使用字符数组存储字符串，数组大小会自动根据字符串长度计算
const char *ptr = data;     // 将指针指向字符数组

//执行小程序
#include "cjson_init.h"
#include <hi_task.h>
#include <string.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
#define ONE_SECOND                          (1000)
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="
#define WECHAT_SUBSCRIBE_LIGHT              "light"
#define WECHAT_SUBSCRIBE_MOVE               "move"
#define WECHAT_SUBSCRIBE_LIGHT_ON_STATE     "1"
#define WECHAT_SUBSCRIBE_LIGHT_OFF_STATE    "0"
static char face = 0;

//执行温湿度传感
#include "dht11.h"
#define DH_SENSOR_GPIO         11// for hispark_pegasus
#define NUM 1
#define DHT11_GPIO DH_SENSOR_GPIO									   
#define	DHT11_DQ_OUT_High IoTGpioSetOutputVal(DHT11_GPIO, 1); //设置GPIO输出高电平
#define	DHT11_DQ_OUT_Low IoTGpioSetOutputVal(DHT11_GPIO, 0); //设置GPIO输出低电平  
uint8_t* temp,humi;

//执行红外传感器
#include "iot_adc.h"

//看门狗
#include "hi_watchdog.h"


osThreadId_t newThread(char *name, osThreadFunc_t func, int*arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024*10, osPriorityNormal, 0, 0
    };
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL) {
        printf("[Thread Test] osThreadNew(%s) failed.\r\n", name);
    } else {
        printf("[Thread Test] osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

//线程测试：该函数会打印自己的参数，对全局变量count进行循环+1操作，之后打印count的值
void threadTest(int *arg)
{
    static int count = 0;
    printf("%s\r\n", (char *)arg);
    osThreadId_t tid = osThreadGetId();
    printf("[Thread Test] threadTest osThreadGetId, thread id:%p\r\n", tid);
    while (1) {
        count++;
        printf("[Thread Test] threadTest, count: %d.\r\n", count);
        osDelay(OS_DELAYONE);
    }
}

//串口主函数
static void UartTask(void)
{
    uint32_t count = 0;
    uint32_t len = 0;
    unsigned char uartReadBuff[UART_BUFF_SIZE] = {0}; //存储ASCII 码值。如果以 %c 输出，会根据ASCII码表转换成对应的字符，如果以 %d 输出，那么还是整数
    

    Uart1GpioInit();
    Uart1Config();

    while (1) {
        // 通过UART1 发送数据 Send data through UART1
        if ((count<=50)&&(data[2]!=0000))
        {
          IoTUartWrite(HI_UART_IDX_1, (unsigned char*)data, strlen(data));
          count++;
        }else if (count>50){
            data[0] ="0";
            data[1] ="0";
            data[2] ="0";
            data[3] ="0";
            count=0;
        }
        // 通过UART1 接收数据 Receive data through UART1
        len = IoTUartRead(HI_UART_IDX_1, uartReadBuff, UART_BUFF_SIZE);
        if ((len > 0)&&(uartReadBuff!=0)) {
            printf(uartReadBuff);
            face=uartReadBuff[3];
            
            //printf(face);
            /*if((uartReadBuff[0]==0xaa)&&(uartReadBuff[1]==0x55)&&(uartReadBuff[3]==0xff)&&(uartReadBuff[2]==0001)){
                face=1;
                printf("111111111111111111111");
            }
            else if ((uartReadBuff[0]==0xaa)&&(uartReadBuff[1]==0x55)&&(uartReadBuff[3]==0xff)&&(uartReadBuff[2]==0002))
            {
                face=2;
                printf("2222222222222222222222222");
            }
            else if ((uartReadBuff[0]==0xaa)&&(uartReadBuff[1]==0x55)&&(uartReadBuff[3]==0xff)&&(uartReadBuff[2]==0003))
            {
                face=3;
                printf("333333333333333333333");
            }*/  
        }
        usleep(U_SLEEP_TIME);
    }
}

void printHex(const unsigned char *data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

//小程序函数
int g_ligthStatus = -1;
typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool  stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack    msgCallBack;
}FunctionCallback;
FunctionCallback g_functinoCallback;

static void DeviceConfigInit(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

static int  DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_functinoCallback.msgCallBack = msgCallBack;
    return 0;
}

static void wechatControlDeviceMsg(hi_gpio_value val)
{
    DeviceConfigInit(val);
}

// < this is the callback function, set to the mqtt, and if any messages come, it will be called
// < The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* 云端下发命令后，板端的操作处理 */
    if (strstr(payload, WECHAT_SUBSCRIBE_MOVE) != NULL) {
        IOT_LOG_DEBUG("接收MOVE成功");
        data[0]=0xAA;
        data[1]=0x55;
        data[2]=0002;// 在这里修改发送的对象
        data[3]=0xFF;
    }

    if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT) != NULL) {
        if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT_OFF_STATE) != NULL) {
            wechatControlDeviceMsg(HI_GPIO_VALUE1);
            g_ligthStatus = HI_FALSE;
        } else {
            wechatControlDeviceMsg(HI_GPIO_VALUE0);
            g_ligthStatus = HI_TRUE;
        }
        IOT_LOG_DEBUG("接收LIGHT成功");
        data[0]=0xAA;
        data[1]=0x55;
        data[2]=0003;// 在这里修改发送的对象
        data[3]=0xFF;
    }
    return HI_NULL;
}

/* publish sample */
hi_void IotPublishSample(void)
{
    /* reported attribute */
    WeChatProfile weChatProfile = {
        .subscribeType = "type",
        .status.subState = "state",
        .status.subReport = "reported",
        .status.reportVersion = "version",
        .status.Token = "clientToken",
        /* report motor */
        .reportAction.subDeviceActionMotor = "motor",
        .reportAction.motorActionStatus = 0, /* 0 : motor off */
        /* report temperature */
        .reportAction.subDeviceActionTemperature = "temperature",
        .reportAction.temperatureData = temp, /* 30 :temperature data */
        /* report humidity */
        .reportAction.subDeviceActionHumidity = "humidity",
        .reportAction.humidityActionData = humidity, /* humidity data */
        
        .reportAction.subDeviceActiontestData="test",
        .reportAction.testData=face-48,//在这里修改上传的人脸数据
    };

    /* report light */
    if (g_ligthStatus == HI_TRUE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 1; /* 1: light on */
    } else if (g_ligthStatus == HI_FALSE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    } else {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    }
    /* profile report */
    IoTProfilePropertyReport(CONFIG_USER_ID, &weChatProfile);
}

static hi_void *DemoEntry(const char *arg)
{
    WifiStaReadyWait();
    cJsonInit();
    IoTMain();
    /* 云端下发回调 */
    IoTSetMsgCallback(DemoMsgRcvCallBack);
    /* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1) {
        IotPublishSample(); // 发布例程
#endif
        TaskMsleep(ONE_SECOND);
    }
    return NULL;
}

//温湿度传感
static void DhtTask(const int *arg)
{   
    
    (void)arg;
    printf("DhtTask start\r\n");
    // 配置GPIO引脚号和输出值
    DHT11_Init();
    while (NUM)
    {   
        DHT11_Read_Data(&temp,&humi);/* code */
        printf("temp:%d,humi:%d\r\n",temp,humi);
    }
}

//红外线传感器
static void ADCLightTask(int *arg)
{
    (void)arg;
    unsigned int ret = 0;
    unsigned int data_temp = 0;
    int count=0;

    while (NUM) {
        // 光敏电阻对应的是ADC channel 4，
        ret = AdcRead(IOT_ADC_CHANNEL_0, &data_temp, IOT_ADC_EQU_MODEL_4, IOT_ADC_CUR_BAIS_DEFAULT, 0xff);
        if (ret != IOT_SUCCESS) {
            printf("ADC Read Fail \r\n");
            return;
        } else {
            /* vlt * 1.8 * 4 / 4096.0 为将码字转换为电压 */
            
            if((data_temp/1000==1)&&(count<=10)){
                count++;
            }
            else if (count>10)
            {
                count=0;
                data[0]=0xAA;
                data[1]=0x55;
                data[2]=0001;// 在这里修改发送的对象
                data[3]=0xFF;
                printf("red_infrad");
            }
            
        }
        TaskMsleep(50); /* 20:sleep 20ms */
    }
}



/*线程操作函数
打印相关信息：
	const char *t_name = osThreadGetName(uart);
    printf("[Thread Test] osThreadGetName, thread name: %s.\r\n", t_name);

    osThreadState_t state = osThreadGetState(uart);
    printf("[Thread Test] osThreadGetState, state :%d.\r\n", state);

    osStatus_t status = osThreadSetPriority(uart, osPriorityNormal4);
    printf("[Thread Test] osThreadSetPriority, status: %d.\r\n", status);

    osPriority_t pri = osThreadGetPriority(uart);
    printf("[Thread Test] osThreadGetPriority, priority: %d.\r\n", pri);

挂起指定线程的运行
	status = osThreadSuspend(uart); 
    printf("[Thread Test] osThreadSuspend, status: %d.\r\n", status);

恢复指定线程的运行
	status = osThreadResume(uart); 
	printf("[Thread Test] osThreadResume, status: %d.\r\n", status);

获取指定线程的栈空间大小	
    uint32_t stacksize = osThreadGetStackSize(uart);
    printf("[Thread Test] osThreadGetStackSize, stacksize: %u.\r\n", stacksize);

获取指定线程的未使用的栈空间大小
    uint32_t stackspace = osThreadGetStackSpace(uart);
    printf("[Thread Test] osThreadGetStackSpace, stackspace: %u.\r\n", stackspace);

获取活跃线程数
    uint32_t t_count = osThreadGetCount();
    printf("[Thread Test] osThreadGetCount, count: %u.\r\n", t_count);

终止指定线程的运行
    osDelay(OS_DELAY);
    status = osThreadTerminate(uart);
    printf("[Thread Test] osThreadTerminate, status: %d.\r\n", status);

*/

/*主线程：thread_main
1.串口线程：thread_uart，一直开启，帧头0xAA,OX55，数据4位，帧尾0xFF
    0001代表红外传感器被遮挡，0002代表小程序端发送move信号（寻找H宝），0003代表小程序端发送light信号（欢迎回家）
2.小程序通信线程：
3.读取红外线程：
4.读取温湿度线程：thread_readTemp，只在上电时开启，测量结束后关闭
*/
void thread_main(int *arg)
{
    (void)arg;
    osThreadId_t uart = newThread("thread_uart",UartTask, "创建线程1"); 
    osThreadId_t readTemp = newThread("thread_readTemp", DhtTask, "创建线程4"); 
    hi_udelay(800000);
    hi_udelay(800000);
    osStatus_t status;
    status = osThreadTerminate(readTemp);
    
    osThreadId_t readRed = newThread("thread_readRed", ADCLightTask, "创建线程3"); 
    osThreadId_t report = newThread("thread_report", DemoEntry, "创建线程2"); 
    
}

static void ThreadTestTask(void)
{
    osThreadAttr_t attr;
    attr.name = "thread_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;
    IoTWatchDogDisable();
    if (osThreadNew((osThreadFunc_t)thread_main, NULL, &attr) == NULL) {
        printf("[ThreadTestTask] Failed to create thread_main!\n");
    }
}

APP_FEATURE_INIT(ThreadTestTask);