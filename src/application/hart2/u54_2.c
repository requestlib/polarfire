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
    while(1U);

  /* Never return */
}

