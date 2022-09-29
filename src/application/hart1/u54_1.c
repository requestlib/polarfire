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
#define OP_NR_MAIL 20

static struct rt_mailbox mb1;
static char mb_str1[] = "first mail.";
static char mb_str3[] = "over";
static char mb_pool1[128];

static char thread1_stack[10000];
static struct rt_thread thread1;
static char thread3_stack[10000];
static struct rt_thread thread3;

/* 线程 3 入口 发送邮件到邮箱1*/
static void thread3_entry(void *parameter)
{
    for(int i=0;i<OP_NR_MAIL;i++) rt_mb_send(&mb1, (rt_uint32_t)&mb_str1);
    rt_mb_send(&mb1, (rt_uint32_t)&mb_str3);
}


/* 线程 1 接收邮箱1的内容 */
static void thread1_entry(void *parameter)
{
    char *str;
    int count=0;
    int start = read_csr(mcycle);
    while (1)
    {
        if (rt_mb_recv(&mb1, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            count++;
            if(str == mb_str3)break;
        }

    }
    int end = read_csr(mcycle);
    rt_kprintf("mailbox tick:%d\n",end-start);
    rt_mb_detach(&mb1);
}



void u54_1(void)
{
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \n", core);

    rt_err_t result = rt_mb_init(&mb1,"mbt1",&mb_pool1[0],sizeof(mb_pool1) / 4, RT_IPC_FLAG_FIFO);   //初始化邮箱1 

    //thread1
    rt_thread_init(&thread1,"thread1",thread1_entry,RT_NULL,&thread1_stack[0],sizeof(thread1_stack),THREAD_PRIORITY, THREAD_TIMESLICE);
    thread1.bind_cpu=1;
    rt_thread_startup(&thread1);

    rt_thread_init(&thread3,"thread3",thread3_entry,RT_NULL,&thread3_stack[0],sizeof(thread3_stack),THREAD_PRIORITY, THREAD_TIMESLICE);
    thread3.bind_cpu=2;
    rt_thread_startup(&thread3);
    
    return 0;
}