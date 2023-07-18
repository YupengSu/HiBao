#ifndef UART_EXAMPLE_H_INCLUDED
#define UART_EXAMPLE_H_INCLUDED

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"

void Uart1GpioInit(void);
void Uart1Config(void);
static void UartTask(void);

#endif // UART_EXAMPLE_H_INCLUDED