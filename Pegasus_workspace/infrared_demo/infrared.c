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
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "iot_errno.h"

#define NUM 1
#define STACK_SIZE 4096
#define ADC_RESOLUTION 1000

static void ADCLightTask(int *arg)
{
    (void)arg;
    unsigned int ret = 0;
    unsigned int data = 0;

    while (NUM) {
        // 光敏电阻对应的是ADC channel 4，
        ret = AdcRead(IOT_ADC_CHANNEL_0, &data, IOT_ADC_EQU_MODEL_4, IOT_ADC_CUR_BAIS_DEFAULT, 0xff);
        if (ret != IOT_SUCCESS) {
            printf("ADC Read Fail \r\n");
            return;
        } else {
            /* vlt * 1.8 * 4 / 4096.0 为将码字转换为电压 */
            printf("ADC Read value is %d \r\n", data/1000);
        }
        TaskMsleep(20); /* 20:sleep 20ms */
    }
}

static void ADCLightDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "ADCLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(ADCLightTask, NULL, &attr) == NULL) {
        printf("[ADCLightDemo] Failed to create ADCLightTask!\n");
    }
}

APP_FEATURE_INIT(ADCLightDemo);
