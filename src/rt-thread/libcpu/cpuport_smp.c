/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/12/23     Bernard      The first version
 * 2018/12/27     Jesven       Add secondary cpu boot
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>

#include "board.h"
#include "mpfs_hal/mss_hal.h"

#ifdef RT_USING_SMP

int rt_hw_cpu_id(void)
{
    return read_csr(mhartid);
}

void rt_hw_ipi_send(int ipi_vector, unsigned int cpu_mask)
{
    int idx;

    for (idx = 0; idx < RT_CPUS_NR; idx ++)
    {
        if (cpu_mask & (1 << idx))
        {
            //clint_ipi_send(idx);
            int a=1;
        }
    }
}


extern void rt_hw_scondary_interrupt_init(void);
extern int rt_hw_tick_init(void);
extern int rt_hw_clint_ipi_enable(void);

void secondary_cpu_c_start(void)
{
    // rt_spin_lock(&_cpus_lock);

    // rt_hw_scondary_interrupt_init();

    // rt_hw_tick_init();

    // rt_hw_clint_ipi_enable();

    // rt_application_init();

    // rt_system_scheduler_start();
}

void rt_hw_secondary_cpu_idle_exec(void)
{
    asm volatile ("nop");
}
#endif /*RT_USING_SMP*/
