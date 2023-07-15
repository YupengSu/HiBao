//线程参考：thread_demo/thread.c

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

//执行线程的位置
#include "uart_control.h"

#define STACK_SIZE 1024
#define OS_DELAY 100
#define OS_DELAYONE 20

osThreadId_t newThread(char *name, osThreadFunc_t func, int*arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024*2, osPriorityNormal, 0, 0
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
1.串口线程：thread_uart
2.小程序通信线程：
3.读取红外线程：
4.读取温湿度线程：
*/
void thread_main(int *arg)
{
    (void)arg;
    osThreadId_t uart = newThread("thread_uart", UartTask, "创建线程1"); 
    osThreadId_t report = newThread("thread_report", threadTest, "创建线程2"); 
    osThreadId_t readRed = newThread("thread_readRed", threadTest, "创建线程3"); 
    osThreadId_t readTemp = newThread("thread_readTemp", threadTest, "创建线程4");

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

    if (osThreadNew((osThreadFunc_t)thread_main, NULL, &attr) == NULL) {
        printf("[ThreadTestTask] Failed to create thread_main!\n");
    }
}

APP_FEATURE_INIT(ThreadTestTask);





