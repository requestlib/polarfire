/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Application code running on U54_1
 *
 * Example project demonstrating the use of polled and interrupt driven
 * transmission and reception over MMUART. Please refer README.md in the root
 * folder of this example project
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "inc/common.h"

/******************************************************************************
 * Instruction message. This message will be transmitted over the UART when
 * the program starts.
 *****************************************************************************/

const uint8_t g_message1[] =
        "\r\n\r\n\r\n **** PolarFire SoC MSS MMUART example ****\r\n\r\n\r\n";

const uint8_t g_message2[] =
        "This program is run from u54_1\r\n\
        \r\n\
Type 0  Show hart 1 debug message\r\n\
Type 1  Show this menu\r\n\
Type 2  Send message using polled method\r\n\
Type 3  send message using interrupt method\r\n\
";

const uint8_t polled_message[] =
        "This message has been transmitted using polled method. \r\n";

const uint8_t intr_message[] =
        " This message has been transmitted using interrupt method. \r\n";


#define RX_BUFF_SIZE    64U
uint8_t g_rx_buff[RX_BUFF_SIZE] = { 0 };
volatile uint32_t count_sw_ints_h1 = 0U;
volatile uint8_t g_rx_size = 0U;
static volatile uint32_t irq_cnt = 0;
uint8_t info_string[100];

/* This is the handler function for the UART RX interrupt.
 * In this example project UART0 local interrupt is enabled on hart0.
 */
void uart1_rx_handler(mss_uart_instance_t *this_uart) {
    uint32_t hart_id = read_csr(mhartid);
    int8_t info_string[50];

    irq_cnt++;
    sprintf(info_string, "UART1 Interrupt count = 0x%x \r\n\r\n", irq_cnt);

    /* This will execute when interrupt from hart 1 is raised */
    g_rx_size = MSS_UART_get_rx(this_uart, g_rx_buff, sizeof(g_rx_buff));

    MSS_UART_polled_tx(&g_mss_uart1_lo, info_string,
            strlen(info_string));
}

/* Main function for the hart1(U54 processor).
 * Application code running on hart1 is placed here.
 * MMUART1 local interrupt is enabled on hart1.
 * In the respective U54 harts, local interrupts of the corresponding MMUART
 * are enabled. e.g. in U54_1.c local interrupt of MMUART1 is enabled. */

const uint8_t g_message34[] =
        " \r\n\r\n------------------------------------\
---------------------------------\r\n\r\n\
 Please observe UART1, as application is using UART1 as \
 User-Interface\r\n\r\n--------------------------------\
-------------------------------------\r\n";

void u54_1(void) {
       volatile uint32_t icount = 0U;
    uint64_t hartid = read_csr(mhartid);


    while (1U)
    {
        icount++;

        if (0x500000U == icount)
        {
            /* Message on uart0 */
            rt_kprintf_uart1("\nu54_1 print:%d:\n%s",1,g_message34);
            // MSS_UART_polled_tx(&g_mss_uart0_lo, fmt, sizeof(fmt));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3, sizeof(g_message3));
            // MSS_UART_polled_tx(&g_mss_uart0_lo, g_message3,sizeof(g_message3));
            icount=0;
        }
    }
}
