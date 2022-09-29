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


#define THREAD_PRIORITY 15
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE 5
#define OP_NR_SEM 200

/* 指向信号量的指针 */
static rt_sem_t dynamic_sem = RT_NULL;
static char thread2_stack[4096];
static struct rt_thread thread2;
static char thread4_stack[4096];
static struct rt_thread thread4;

/* 线程 2 获取信号量 */
static void thread2_entry(void *parameter)
{
    static rt_err_t result;
    static rt_uint8_t number = 0;
    int start = read_csr(mcycle);
    while(number<OP_NR_SEM)
    {
        /* 永久方式等待信号量，获取到信号量，则执行 number 自加的操作 */
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
        number++;
    }
    int end = read_csr(mcycle);
    rt_kprintf("sem:%d\n",end-start);
    while(1);
}

static void thread4_entry(void *parameter)
{
    for(int i=0;i<OP_NR_SEM;i++) rt_sem_release(dynamic_sem);
    while(1);
}

void u54_3(void){
    // 测试1 存在空闲核
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \r\n", core);

    /* 创建一个动态信号量，初始值是 0 */
    dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_PRIO);

    rt_thread_init(&thread2,"thread2",thread2_entry,RT_NULL,&thread2_stack[0],sizeof(thread2_stack),THREAD_PRIORITY, THREAD_TIMESLICE);
    thread2.bind_cpu=3;
    rt_thread_startup(&thread2);

    rt_thread_init(&thread4,"thread4",thread4_entry,RT_NULL,&thread4_stack[0],sizeof(thread4_stack),THREAD_PRIORITY, THREAD_TIMESLICE);
    thread4.bind_cpu=4;
    rt_thread_startup(&thread4);
}


