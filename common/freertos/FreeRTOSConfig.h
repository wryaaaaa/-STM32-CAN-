/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION		1
/* 置 1：使能时间片调度（默认即为1）；
   置 0：关闭时间片调度（同优先级任务不会自动轮转，直到主动放弃或被高优先级抢占） */
#define configUSE_TIME_SLICING      1
#define configUSE_IDLE_HOOK			0
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ			( ( unsigned long ) 72000000 )
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES		( 5 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 17 * 1024 ) )
#define configMAX_TASK_NAME_LEN		( 16 )
#define configUSE_TRACE_FACILITY	1
#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		1

/* 堆栈溢出检测配置
   0 = 禁用
   1 = 方法1 - 仅检测任务栈溢出
   2 = 方法2 - 检测任务和中断栈溢出 */
#define configCHECK_FOR_STACK_OVERFLOW	            0

/* 启用统计格式化函数（必须启用才能使用vTaskList等函数） */
#define configUSE_STATS_FORMATTING_FUNCTIONS	  	1
/* 运行时统计配置: 关闭以减小代码体积，启用需提供 vConfigureTimerForRunTimeStats 和 TaskRunTimeCounter */
#define configGENERATE_RUN_TIME_STATS             	0
#define configUSE_STATS_FORMATTING_FUNCTIONS      	0

/* 内存分配失败钩子函数配置 */
#define configUSE_MALLOC_FAILED_HOOK              	0

//添加检测任务堆栈最小剩余空间函数的宏定义
#define INCLUDE_uxTaskGetStackHighWaterMark         1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1                                   //任务挂起宏 配置为1的时候可以使用任务挂起函数
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1

/* 新增API函数宏定义 */
#define INCLUDE_xTaskGetSchedulerState            1   /* 获取调度器状态 */
#define INCLUDE_xTaskGetCurrentTaskHandle         1   /* 获取当前任务句柄 */
#define INCLUDE_eTaskGetState                     1   /* 获取任务状态 */
#define INCLUDE_vTaskList                         1   /* 任务列表函数 */

/* 互斥量和信号量相关配置 */
#define configUSE_MUTEXES                         1   /* 启用互斥量 */
#define configUSE_RECURSIVE_MUTEXES               1   /* 启用递归互斥量 */
#define configUSE_COUNTING_SEMAPHORES             1   /* 启用计数信号量 */

/* 队列和任务通知配置 */
#define configUSE_QUEUE_SETS                      0   /* 启用队列集 */
#define configQUEUE_REGISTRY_SIZE                 8   /* 队列注册表大小 */
#define configUSE_TASK_NOTIFICATIONS              1   /* 启用任务通知 */

/* 事件组配置 */
#define configUSE_EVENT_GROUPS                    1   /* 启用事件组 */

/* 定义 STM32 的优先级位数（4位），确保计算正确 */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS       __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS       4
#endif

/* 最低优先级 15 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15

/* 
 * 关键修改：FreeRTOS 可管理的最高优先级 
 * 设为 5，意味着优先级 5~15 的中断可以被 FreeRTOS 屏蔽/管理。
 * 0~4 的中断 FreeRTOS 管不了。
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* 
 * 下面这两个宏通过移位自动计算，适配 STM32 的高位对齐特性
 * 这样写比直接写 191 或 0xB0 更安全、更标准
 */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

#define xPortPendSVHandler 		PendSV_Handler
#define xPortSysTickHandler 	SysTick_Handler
#define vPortSVCHandler 		SVC_Handler

/* 断言配置 - 调试时启用 正式产品需要将其注释掉 */
#ifdef DEBUG
    #define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#else
    #define configASSERT( x )
#endif

#endif /* FREERTOS_CONFIG_H */

