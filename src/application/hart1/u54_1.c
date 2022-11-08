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

#define THREAD_PRIORITY 20
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE 500
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_thread_t tid4 = RT_NULL;
// static rt_thread_t tid_moniter = RT_NULL;
static int para1 = 0;

/* 线程1 入口函数*/
void thread_entry(void *parameter)
{
    int count=0;
    while(rt_tick_get()<300000){
        primary_cal_test(1);
        count++;
    }
    rt_kprintf("count:%d \r\n",count);
}

void moniter_thread_entry(void *parameter){
    int count=0;
    while(1){
        rt_thread_delay(2000);
        for(int i=1;i<RT_CPUS_NR;i++){
            rt_kprintf("cpu[%d] usage:",i);
            rt_kprintf(" %d%%", (int)get_cpu_usage_float(i));
            rt_kprintf("   total_tick:%d, idle_tick:%d\n",rt_cpu_index(i)->recent_total_ticks,rt_cpu_index(i)->idle_ticks);
        }
        list_thread();
    }
}

void u54_1(void){
    // 测试1 存在空闲核
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \n", core);
    tid1 = rt_thread_create("thread1", thread_entry, NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    tid2 = rt_thread_create("thread2", thread_entry, NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    tid3 = rt_thread_create("thread3", thread_entry, NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    tid4 = rt_thread_create("thread4", thread_entry, NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    tid4->bind_cpu=4;
    tid3->bind_cpu=3;
    tid2->bind_cpu=2;
    tid1->bind_cpu=1;
    rt_thread_startup(tid4);
    rt_thread_startup(tid3);
    rt_thread_startup(tid2);
    rt_thread_startup(tid1);

    // tid_moniter = rt_thread_create("moniter", moniter_thread_entry, RT_NULL, THREAD_STACK_SIZE*4, THREAD_PRIORITY, THREAD_TIMESLICE);
    // if(tid_moniter != RT_NULL){
    //     tid_moniter->bind_cpu=1;
    // }
    // rt_thread_startup(tid_moniter);
    // rt_thread_startup(tid1);
    // while(1);

}



