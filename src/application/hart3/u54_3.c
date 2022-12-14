/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Application code running on U54_3
 *
 */
#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include<rtthread.h>
#include<board.h>
#include<rthw.h>
#include<test_tool.h>


#define THREAD_PRIORITY 10
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE 5
static rt_thread_t tid3 = RT_NULL;

/* 线程1 入口函数*/
void thread_entry3(void *parameter)
{
    int cal_num = 80000;
    while(1){
        primary_cal_test(cal_num);
        rt_thread_delay(1000);
        rt_kprintf("thread3\n\n");
    }
}

void u54_3(void){
    // 测试1 存在空闲核
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \n", core);

    tid3 = rt_thread_create("thread3", thread_entry3, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY+10, THREAD_TIMESLICE);
    if(tid3 != RT_NULL){
        tid3->bind_cpu = 3;
        rt_thread_startup(tid3);
    }

}


