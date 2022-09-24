/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-09-20     Bernard      Change the name to components.c
 *                             And all components related header files.
 * 2012-12-23     Bernard      fix the pthread initialization issue.
 * 2013-06-23     Bernard      Add the init_call for components initialization.
 * 2013-07-05     Bernard      Remove initialization feature for MS VC++ compiler
 * 2015-02-06     Bernard      Remove the MS VC++ support and move to the kernel
 * 2015-05-04     Bernard      Rename it to components.c because compiling issue
 *                             in some IDEs.
 * 2015-07-29     Arda.Fu      Add support to use RT_USING_USER_MAIN with IAR
 * 2018-11-22     Jesven       Add secondary cpu boot up
 */

#include <rthw.h>
#include <rtthread.h>
#include <mpfs_hal/common/encoding.h>
#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/common/nwc/mss_nwc_init.h"

#ifdef RT_USING_USER_MAIN
#ifndef RT_MAIN_THREAD_STACK_SIZE
#define RT_MAIN_THREAD_STACK_SIZE     2048
#endif /* RT_MAIN_THREAD_STACK_SIZE */
#ifndef RT_MAIN_THREAD_PRIORITY
#define RT_MAIN_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX / 3)
#endif /* RT_MAIN_THREAD_PRIORITY */
#endif /* RT_USING_USER_MAIN */

#ifdef RT_USING_COMPONENTS_INIT
/*
 * Components Initialization will initialize some driver and components as following
 * order:
 * rti_start         --> 0
 * BOARD_EXPORT      --> 1
 * rti_board_end     --> 1.end
 *
 * DEVICE_EXPORT     --> 2
 * COMPONENT_EXPORT  --> 3
 * FS_EXPORT         --> 4
 * ENV_EXPORT        --> 5
 * APP_EXPORT        --> 6
 *
 * rti_end           --> 6.end
 *
 * These automatically initialization, the driver or component initial function must
 * be defined with:
 * INIT_BOARD_EXPORT(fn);
 * INIT_DEVICE_EXPORT(fn);
 * ...
 * INIT_APP_EXPORT(fn);
 * etc.
 */
static int rti_start(void)
{
    return 0;
}
INIT_EXPORT(rti_start, "0");

static int rti_board_start(void)
{
    return 0;
}
INIT_EXPORT(rti_board_start, "0.end");

static int rti_board_end(void)
{
    return 0;
}
INIT_EXPORT(rti_board_end, "1.end");

static int rti_end(void)
{
    return 0;
}
INIT_EXPORT(rti_end, "6.end");

/**
 * @brief  Onboard components initialization. In this function, the board-level
 *         initialization function will be called to complete the initialization
 *         of the on-board peripherals.
 */
void rt_components_board_init(void)
{
#if RT_DEBUG_INIT
    int result;
    const struct rt_init_desc *desc;
    for (desc = &__rt_init_desc_rti_board_start; desc < &__rt_init_desc_rti_board_end; desc ++)
    {
        rt_kprintf_uart1("initialize %s", desc->fn_name);
        result = desc->fn();
        rt_kprintf_uart1(":%d done\n", result);
    }
#else
    volatile const init_fn_t *fn_ptr;

    for (fn_ptr = &__rt_init_rti_board_start; fn_ptr < &__rt_init_rti_board_end; fn_ptr++)
    {
        (*fn_ptr)();
    }
#endif /* RT_DEBUG_INIT */
}

/**
 * @brief  RT-Thread Components Initialization.
 */
void rt_components_init(void)
{
#if RT_DEBUG_INIT
    int result;
    const struct rt_init_desc *desc;

    rt_kprintf_uart1("do components initialization.\n");
    for (desc = &__rt_init_desc_rti_board_end; desc < &__rt_init_desc_rti_end; desc ++)
    {
        rt_kprintf_uart1("initialize %s", desc->fn_name);
        result = desc->fn();
        rt_kprintf_uart1(":%d done\n", result);
    }
#else
    volatile const init_fn_t *fn_ptr;

    for (fn_ptr = &__rt_init_rti_board_end; fn_ptr < &__rt_init_rti_end; fn_ptr ++)
    {
        (*fn_ptr)();
    }
#endif /* RT_DEBUG_INIT */
}
#endif /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_USER_MAIN

void rt_application_init(void);
//void rt_hw_board_init(HLS_DATA* hls);
int rtthread_startup(void);
static bool main_hart_done = false;

#ifdef __ARMCC_VERSION
extern int $Super$$main(void);
/* re-define main function */
int $Sub$$main(void)
{
    // rtthread_startup(hls);
    return 0;
}
#elif defined(__ICCARM__)
extern int main(void);
/* __low_level_init will auto called by IAR cstartup */
extern void __iar_data_init3(void);
int __low_level_init(void)
{
    // call IAR table copy function.
    __iar_data_init3();
    // rtthread_startup();
    return 0;
}
#elif defined(__GNUC__)
/* Add -eentry to arm-none-eabi-gcc argument */
int primary_cpu_entry(HLS_DATA* hls)
{
    int level = rt_hw_local_irq_disable();
    rt_hw_board_init(hls);
    rt_show_version();
    rt_system_timer_init();
    rt_system_scheduler_init();
    rt_application_init();
    rt_hw_local_irq_enable(level);
     main_hart_done = true;
    while(1);
    return 0;
}
int other_cpu_entry(HLS_DATA* hls)
{
    while(1) if(main_hart_done)break;
    rt_hw_board_init_other(hls);
    //list_thread();
    rt_system_scheduler_start();
    while(1);
    return 0;
}
#endif

#ifndef RT_USING_HEAP
/* if there is not enable heap, we should use static thread and stack. */
ALIGN(8)
static rt_uint8_t main_stack[RT_MAIN_THREAD_STACK_SIZE];
struct rt_thread main_thread;
#endif /* RT_USING_HEAP */

/**
 * @brief  The system main thread. In this thread will call the rt_components_init()
 *         for initialization of RT-Thread Components and call the user's programming
 *         entry main().
 */
void main_thread_entry(void *parameter)
{
    extern void e51(void);
    extern void u54_1(void);
    extern void u54_2(void);
    extern void u54_3(void);
    extern void u54_4(void);
    int hart_id = rt_hw_cpu_id();

#ifdef RT_USING_COMPONENTS_INIT
    /* RT-Thread components initialization */
    rt_components_init();
#endif /* RT_USING_COMPONENTS_INIT */

    /* invoke system main function */
#ifdef __ARMCC_VERSION
    {
        extern int $Super$$main(void);
        $Super$$main(); /* for ARMCC. */
    }
#elif defined(__ICCARM__) || defined(__GNUC__) || defined(__TASKING__)
    volatile uint64_t dummy;
    switch(hart_id)
    {
        case 0U:
            e51();
            break;
        case 1U:
            (void)init_pmp((uint8_t)1);
            u54_1();
            break;
        case 2U:
            (void)init_pmp((uint8_t)2);
            u54_2();
            break;
        case 3U:
            (void)init_pmp((uint8_t)3);
            u54_3();
            break;
        case 4U:
            (void)init_pmp((uint8_t)4);
            u54_4();
            break;
        default:
            break;
    }
#endif
}

/**
 * @brief  This function will create and start the main thread, but this thread
 *         will not run until the scheduler starts.
 */
void rt_application_init(void)
{
    rt_thread_t tid1;
    rt_thread_t tid2;
    rt_thread_t tid3;
    rt_thread_t tid4;
    //core1 main
    tid1 = rt_thread_create("main", main_thread_entry, RT_NULL,
                           RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(tid1 != RT_NULL);
    tid1->bind_cpu = 1;
    rt_thread_startup(tid1);
    
    //core2 main
    tid2 = rt_thread_create("main", main_thread_entry, RT_NULL,
                           RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(tid2 != RT_NULL);
    tid2->bind_cpu = 2;
    rt_thread_startup(tid2);

    //core3 main
    tid3 = rt_thread_create("main", main_thread_entry, RT_NULL,
                           RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(tid3 != RT_NULL);
    tid3->bind_cpu = 3;
    rt_thread_startup(tid3);

    //core4 main
    tid4 = rt_thread_create("main", main_thread_entry, RT_NULL,
                           RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(tid4 != RT_NULL);
    tid4->bind_cpu = 4;
    rt_thread_startup(tid4);
    
}

/**
 * @brief  This function will call all levels of initialization functions to complete
 *         the initialization of the system, and finally start the scheduler.
 */
int rtthread_startup(void)
{

    /* board level initialization
     * NOTE: please initialize heap inside board initialization.
     */
    /* create init_thread */
    rt_application_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    int icount=0;
    while (1)
    {
        icount++;

        if (0x500000U == icount)
        {
            /* Message on uart0 */
            rt_kprintf_uart1("\ncontent%d:\n%s",1,"looping");
            // MSS_UART_polled_tx(&g_mss_uart0_lo, fmt, sizeof(fmt));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3, sizeof(g_message3));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3,sizeof(g_message3));
            icount=0;
        }
    }
    
    return 0;
}
#endif /* RT_USING_USER_MAIN */
