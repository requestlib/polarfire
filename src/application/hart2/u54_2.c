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


void u54_2(void)
{
    int core = rt_hw_cpu_id();
    rt_kprintf("Core %d Hello world \r\n", core);
    while(1);
}



