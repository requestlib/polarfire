/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Application code running on e51
 *
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include <rtthread.h>
volatile uint32_t count_sw_ints_h0 = 0U;

/* Main function for the hart0(e51 processor).
 * Application code running on hart0 is placed here
 *
 * The hart1 goes into WFI. hart0 brings it out of WFI when it raises the first
 * Software interrupt.
 */
const uint8_t g_message3[] =
        " \r\n\r\n------------------------------------\
---------------------------------\r\n\r\n\
 Please observe UART1, as application is using UART1 as \
 User-Interface\r\n\r\n--------------------------------\
-------------------------------------\r\n";

void e51(void)
{
    volatile uint32_t icount = 0U;
    uint64_t hartid = read_csr(mhartid);


    while (1U)
    {
        icount++;

        if (0x500000U == icount)
        {
            /* Message on uart0 */
            rt_kprintf_uart1("content%d:\n%s",1,g_message3);
            // MSS_UART_polled_tx(&g_mss_uart0_lo, fmt, sizeof(fmt));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3, sizeof(g_message3));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3,sizeof(g_message3));
            icount=0;
        }
    }

    /* never return */
}

