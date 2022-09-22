/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 *2021-06-10      xiaoyu       implement rt_hw_us_delay()
 */
#include <stddef.h>
#include <stdbool.h>
#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/common/nwc/mss_nwc_init.h"
#include "board.h"

void rt_hw_board_init(HLS_DATA* hls){
    uint8_t hart_id;
    ptrdiff_t stack_top;

    config_l2_cache();
    init_memory();
    hls->my_hart_id = MPFS_HAL_FIRST_HART;
    load_virtual_rom();
    (void)init_bus_error_unit();
    (void)init_mem_protection_unit();
    (void)init_pmp((uint8_t)MPFS_HAL_FIRST_HART);
    (void)mss_set_apb_bus_cr((uint32_t)LIBERO_SETTING_APBBUS_CR);
    (void)mss_nwc_init();

    /* main hart init's the PLIC */
    PLIC_init_on_reset();
    stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h0$);
    hls = (HLS_DATA*)(stack_top - HLS_DEBUG_AREA_SIZE);
    hls->in_wfi_indicator = HLS_MAIN_HART_STARTED;
    hls->my_hart_id = MPFS_HAL_FIRST_HART;
    WFI_SM sm_check_thread = INIT_THREAD_PR;
    hart_id = MPFS_HAL_FIRST_HART + 1U;
    while( hart_id <= MPFS_HAL_LAST_HART)
    {
        uint32_t wait_count = 0U;

        switch(sm_check_thread)
        {
            default:
            case INIT_THREAD_PR:

                switch (hart_id)
                {
                    case 1:
                        stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h1$);
                        break;
                    case 2:
                        stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h2$);
                        break;
                    case 3:
                        stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h3$);
                        break;
                    case 4:
                        stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h4$);
                        break;
                }
                hls = (HLS_DATA*)(stack_top - HLS_DEBUG_AREA_SIZE);
                sm_check_thread = CHECK_WFI;
                wait_count = 0U;
                break;

            case CHECK_WFI:
                if( hls->in_wfi_indicator == HLS_OTHER_HART_IN_WFI )
                {
                    /* Separate state- to add a little delay */
                    sm_check_thread = SEND_WFI;
                }
                break;

            case SEND_WFI:
                hls->my_hart_id = hart_id; /* record hartid locally */
                raise_soft_interrupt(hart_id);
                sm_check_thread = CHECK_WAKE;
                wait_count = 0UL;
                break;

            case CHECK_WAKE:
                if( hls->in_wfi_indicator == HLS_OTHER_HART_PASSED_WFI )
                {
                    sm_check_thread = INIT_THREAD_PR;
                    hart_id++;
                    wait_count = 0UL;
                }
                else
                {
                    wait_count++;
                    if(wait_count > 0x10U)
                    {
                        if( hls->in_wfi_indicator == HLS_OTHER_HART_IN_WFI )
                        {
                            hls->my_hart_id = hart_id; /* record hartid locally */
                            raise_soft_interrupt(hart_id);
                            wait_count = 0UL;
                        }
                    }
                }
                break;
        }
    }
    stack_top = (ptrdiff_t)((uint8_t*)&__stack_top_h0$);
    hls = (HLS_DATA*)(stack_top - HLS_DEBUG_AREA_SIZE);
    hls->in_wfi_indicator = HLS_MAIN_HART_FIN_INIT;

    (void)mss_config_clk_rst(MSS_PERIPH_FIC0, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_FIC1, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_FIC2, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_FIC3, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);

    (void)rt_hw_board_init_other(hls);
}


void rt_hw_board_init_other(HLS_DATA* hls){
    extern char __app_stack_top_h0;
    extern char __app_stack_top_h1;
    extern char __app_stack_top_h2;
    extern char __app_stack_top_h3;
    extern char __app_stack_top_h4;

    const uint64_t app_stack_top_h0 = (const uint64_t)&__app_stack_top_h0 - (HLS_DEBUG_AREA_SIZE);
    const uint64_t app_stack_top_h1 = (const uint64_t)&__app_stack_top_h1 - (HLS_DEBUG_AREA_SIZE);
    const uint64_t app_stack_top_h2 = (const uint64_t)&__app_stack_top_h2 - (HLS_DEBUG_AREA_SIZE);
    const uint64_t app_stack_top_h3 = (const uint64_t)&__app_stack_top_h3 - (HLS_DEBUG_AREA_SIZE);
    const uint64_t app_stack_top_h4 = (const uint64_t)&__app_stack_top_h4 - (HLS_DEBUG_AREA_SIZE);

    const uint64_t app_hart_common_start = (const uint64_t)&__app_hart_common_start;
    hls->shared_mem = (uint64_t *)app_hart_common_start;
    hls->shared_mem_marker = SHARED_MEM_INITALISED_MARKER;
    hls->shared_mem_status = SHARED_MEM_DEFAULT_STATUS;

    volatile uint64_t dummy;
    switch(hls->my_hart_id)
    {
        case 0U:
            __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h0));
            e51();
            break;
        case 1U:
            (void)init_pmp((uint8_t)1);
            __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h1));
            u54_1();
            break;
        case 2U:
            (void)init_pmp((uint8_t)2);
            __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h2));
            u54_2();
            break;
        case 3U:
            (void)init_pmp((uint8_t)3);
            __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h3));
            u54_3();
            break;
        case 4U:
            (void)init_pmp((uint8_t)4);
            __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h4));
            u54_4();
            break;
        default:
            break;
    }
}


/*==============================================================================
 * Load the virtual ROM located at address 0x20003120 within the SCB system
 * registers with an executable allowing to park a hart in an infinite loop.
 */
#ifdef  MPFS_HAL_HW_CONFIG
#define VIRTUAL_BOOTROM_BASE_ADDR   0x20003120UL
#define NB_BOOT_ROM_WORDS           8U
const uint32_t rom[NB_BOOT_ROM_WORDS] =
{
    0x00000513U,    /* li a0, 0 */
    0x34451073U,    /* csrw mip, a0 */
    0x10500073U,    /* wfi */
    0xFF5FF06FU,    /* j 0x20003120 */
    0xFF1FF06FU,    /* j 0x20003120 */
    0xFEDFF06FU,    /* j 0x20003120 */
    0xFE9FF06FU,    /* j 0x20003120 */
    0xFE5FF06FU     /* j 0x20003120 */
};

void load_virtual_rom(void)
{
    volatile uint32_t * p_virtual_bootrom = (uint32_t *)VIRTUAL_BOOTROM_BASE_ADDR;
    config_copy( (void *)p_virtual_bootrom, (void *)rom,sizeof(rom[NB_BOOT_ROM_WORDS]));
}
#endif  /* MPFS_HAL_HW_CONFIG */

/*==============================================================================
 * Put the hart executing this code into an infinite loop executing from the
 * SCB system register memory space.
 * This allows preventing a hart from accessing memory regardless of memory
 * hierarchy configuration or compiler/linker settings.
 * This function relies on load_virtual_rom() having been called previously to
 * populate the virtual ROM with a suitable executable.
 */
static void park_hart(void)
{
    clear_csr(mstatus, MSTATUS_MIE);
    __asm volatile("fence.i");
    __asm volatile("li ra,0x20003120");
    __asm volatile("ret");
}

/*==============================================================================
 * E51 code executing after system startup.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
__attribute__((weak)) void e51(void)
{
    /* Put hart in safe infinite WFI loop. */
     park_hart();
}

/*==============================================================================
 * First U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
  */
__attribute__((weak)) void u54_1(void)
{
    /* Put hart in safe infinite WFI loop. */
     park_hart();
}


/*==============================================================================
 * Second U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
__attribute__((weak)) void u54_2(void)
{
    /* Put hart in safe infinite WFI loop. */
    park_hart();
}

/*==============================================================================
 * Third U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
__attribute__((weak)) void u54_3(void)
{
    /* Put hart in safe infinite WFI loop. */
     park_hart();
}

/*==============================================================================
 * Fourth U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
__attribute__((weak)) void u54_4(void)
{
    /* Put hart in safe infinite WFI loop. */
     park_hart();
}

 /*-----------------------------------------------------------------------------
  * _start() function called invoked
  * This function is called on power up and warm reset.
  */
 __attribute__((weak)) void init_memory( void)
 {
    copy_section(&__text_load, &__text_start, &__text_end);
    copy_section(&__sdata_load, &__sdata_start, &__sdata_end);
    copy_section(&__data_load, &__data_start, &__data_end);

    zero_section(&__sbss_start, &__sbss_end);
    zero_section(&__bss_start, &__bss_end);

    __disable_all_irqs();      /* disables local and global interrupt enable */
 }

 /*-----------------------------------------------------------------------------
   * _start() function called invoked
   * This function is called on power up and warm reset.
   */
  __attribute__((weak)) void init_ddr(void)
  {
    if ((LIBERO_SETTING_DDRPHY_MODE & DDRPHY_MODE_MASK) != DDR_OFF_MODE) {
#ifdef DDR_SUPPORT
        uint64_t end_address;

#if 0 /* enable to init cache to zero using 64 bit writes */
        end_address = LIBERO_SETTING_DDR_64_NON_CACHE + LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI2_0 + LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI2_1;
        zero_section((uint64_t *)LIBERO_SETTING_DDR_64_NON_CACHE, (uint64_t *)end_address);
#endif

        end_address = LIBERO_SETTING_DDR_64_CACHE + LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI1_0 + LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI1_1;
        zero_section((uint64_t *)LIBERO_SETTING_DDR_64_CACHE, (uint64_t *)end_address);
#endif
    }
  }

 /**
  * This function is configured by editing parameters in
  * mss_sw_config.h as required.
  * @return
  */

__attribute__((weak)) uint8_t init_bus_error_unit(void)
{
#ifndef SIFIVE_HIFIVE_UNLEASHED
    uint8_t hart_id;
    /* Init BEU in all harts - enable local interrupt */
    for(hart_id = MPFS_HAL_FIRST_HART; hart_id <= MPFS_HAL_LAST_HART; hart_id++)
    {
        BEU->regs[hart_id].ENABLE      = (uint64_t)BEU_ENABLE;
        BEU->regs[hart_id].PLIC_INT    = (uint64_t)BEU_PLIC_INT;
        BEU->regs[hart_id].LOCAL_INT   = (uint64_t)BEU_LOCAL_INT;
        BEU->regs[hart_id].CAUSE       = 0ULL;
        BEU->regs[hart_id].ACCRUED     = 0ULL;
        BEU->regs[hart_id].VALUE       = 0ULL;
    }
#endif
    return (0U);
}

/**
 * init_mem_protection_unit(void)
 * add this function to you code and configure as required
 * @return
 */
__attribute__((weak)) uint8_t init_mem_protection_unit(void)
{
#ifndef SIFIVE_HIFIVE_UNLEASHED
    mpu_configure();
#endif
    return (0U);
}

/**
 * init_pmp(void)
 * add this function to you code and configure as required
 * @return
 */
__attribute__((weak)) uint8_t init_pmp(uint8_t hart_id)
{
    pmp_configure(hart_id);
    return (0U);
}

/**
 * set_apb_bus_cr(void)
 * todo: add check to see if value valid re. mss configurator
 * @return
 */
__attribute__((weak)) uint8_t mss_set_apb_bus_cr(uint32_t reg_value)
{
    SYSREG->APBBUS_CR = reg_value;
    return (0U);
}

/**
 * get_apb_bus_cr(void)
 * @return
 */
__attribute__((weak)) uint8_t mss_get_apb_bus_cr(void)
{
    return (SYSREG->APBBUS_CR);
}

