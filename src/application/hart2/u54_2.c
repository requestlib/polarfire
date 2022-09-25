/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Application code running on U54_2
 *
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "inc/common.h"

volatile uint32_t count_sw_ints_h2 = 0U;

/* Main function for the hart2(U54_2 processor).
 * Application code running on hart4 is placed here
 *
 * The hart2 goes into WFI. hart0 brings it out of WFI when it raises the first
 * Software interrupt to this hart.
 */
void u54_2(void)
{
  const uint8_t g_message3[] =
        " \r\n\r\n------------------------------------\
---------------------------------\r\n\r\n\
 Please observe UART1, as application is using UART1 as \
 User-Interface\r\n\r\n--------------------------------\
-------------------------------------\r\n";
    int icount=0;
    while(1U){
      icount++;

        if (0x1000000U == icount)
        {
            /* Message on uart0 */
            // rt_kprintf("\nu54_2 print:%d:\n%s",1,g_message3);
            // MSS_UART_polled_tx(&g_mss_uart0_lo, fmt, sizeof(fmt));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3, sizeof(g_message3));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3,sizeof(g_message3));
            icount=0;
        }
    }

  /* Never return */
}

