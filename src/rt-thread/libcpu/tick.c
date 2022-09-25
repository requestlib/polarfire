/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/28     Bernard      The unify RISC-V porting code.
 */

#include <rthw.h>
#include <rtthread.h>
#include <mpfs_hal/mss_hal.h>

static volatile unsigned long tick_cycles = 0;
// void handle_m_timer_interrupt(void)
static uint64_t g_systick_increment[5] = {0ULL,0ULL,0ULL,0ULL,0ULL};

int tick_isr(void)
{
    uint64_t hart_id = read_csr(mhartid);
    clear_csr(mie, MIP_MTIP);
    CLINT->MTIMECMP[hart_id] = CLINT->MTIME + g_systick_increment[hart_id];
    rt_tick_increase();
    set_csr(mie, MIP_MTIP);

}

/* Sets and enable the timer interrupt */
int rt_hw_tick_init(void)
{
    /* Read core id */
    volatile rt_uint32_t ret_val = ERROR;

    rt_uint64_t mhart_id = read_csr(mhartid);

    /*
     * We are assuming the tick rate is in milli-seconds
     *
     * convert RTC frequency into milliseconds and multiple by the tick rate
     *
     */

    g_systick_increment[mhart_id] = LIBERO_SETTING_MSS_RTC_TOGGLE_CLK / RT_TICK_PER_SECOND;

    if (g_systick_increment[mhart_id] > 0ULL)
    {

        CLINT->MTIMECMP[mhart_id] = CLINT->MTIME + g_systick_increment[mhart_id];
        
        set_csr(mie, MIP_MTIP);   /* mie Register - Machine Timer Interrupt Enable */
        ret_val = SUCCESS;
    }

    return (ret_val);
}
