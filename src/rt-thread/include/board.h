/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-5-30      Bernard      the first version
 */

#ifndef BOARD_H__
#define BOARD_H__

#include <rtconfig.h>
#include <rtthread.h>
#include <board_defs.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum WFI_SM_
{
    INIT_THREAD_PR                      = 0x00,         /*!< 0 init pointer   */
    CHECK_WFI                           = 0x01,         /*!< is hart in wfi?  */
    SEND_WFI                            = 0x02,         /*!< separate state to
                                                            add a little delay*/
    CHECK_WAKE                          = 0x03,         /*!< has hart left wfi*/
} WFI_SM;

/*------------------------------------------------------------------------------
 * Markers used to indicate startup status of hart
 */
#ifndef HLS_DATA_IN_WFI
#define HLS_DATA_IN_WFI                     0x12345678U
#endif
#ifndef HLS_DATA_PASSED_WFI
#define HLS_DATA_PASSED_WFI                 0x87654321U
#endif

#ifndef SHARED_MEM_INITALISED_MARKER
#define SHARED_MEM_INITALISED_MARKER        0xA1A2A3A4UL
#endif
#ifndef SHARED_MEM_DEFAULT_STATUS
#define SHARED_MEM_DEFAULT_STATUS           0x00000000UL
#endif

typedef struct HLS_DATA_
{
    volatile rt_uint32_t in_wfi_indicator;
    volatile rt_uint32_t my_hart_id;
    volatile rt_uint32_t shared_mem_marker;
    volatile rt_uint32_t shared_mem_status;
    volatile rt_uint64_t * shared_mem;
} HLS_DATA;

/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */
extern unsigned long __stack_top_h0$;
extern unsigned long __stack_bottom_h0$;
extern unsigned long __stack_top_h1$;
extern unsigned long __stack_bottom_h1$;
extern unsigned long __stack_top_h2$;
extern unsigned long __stack_bottom_h2$;
extern unsigned long __stack_top_h3$;
extern unsigned long __stack_bottom_h3$;
extern unsigned long __stack_top_h4$;
extern unsigned long __stack_bottom_h4$;
extern unsigned long __app_hart_common_start;
extern unsigned long __app_hart_common_end;

extern unsigned long __data_load;
extern unsigned long __data_start;
extern unsigned long __data_end;

extern unsigned long __sbss_start;
extern unsigned long __sbss_end;

extern unsigned long __bss_start;
extern unsigned long __bss_end;

extern unsigned long __sdata_load;
extern unsigned long __sdata_start;
extern unsigned long __sdata_end;

extern unsigned long __text_load;
extern unsigned long __text_start;
extern unsigned long __text_end;

extern unsigned long __l2lim_end;

extern unsigned long __e51itim_start;
extern unsigned long __e51itim_end;

extern unsigned long __u54_1_itim_start;
extern unsigned long __u54_1_itim_end;

extern unsigned long __u54_2_itim_start;
extern unsigned long __u54_2_itim_end;

extern unsigned long __u54_3_itim_start;
extern unsigned long __u54_3_itim_end;

extern unsigned long __u54_4_itim_start;
extern unsigned long __u54_4_itim_end;

#ifndef MPFS_HAL_HW_CONFIG
extern unsigned long __uninit_bottom$;
extern unsigned long __uninit_top$;
#endif

// (uint64_t *)LIBERO_SETTING_DDR_64_CACHE, (uint64_t *)end_address
#define RT_HW_HEAP_BEGIN    (void*)LIBERO_SETTING_DDR_64_CACHE
/*
 * Function Declarations
 */
void rt_hw_board_init(HLS_DATA* hls);
void rt_hw_board_init_other(HLS_DATA* hls);
void rt_hw_uart_init(void);
void main_first_hart(HLS_DATA* hls);
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);
void init_memory( void);
rt_base_t init_ddr( void);
rt_uint8_t init_mem_protection_unit(void);
rt_uint8_t init_pmp(rt_uint8_t hart_id);
rt_uint8_t init_bus_error_unit( void);
rt_uint8_t mss_set_apb_bus_cr(rt_uint32_t reg_value);
rt_uint8_t mss_get_apb_bus_cr(void);
char * memfill(void *dest, const void * src, rt_base_t len);
char * config_copy(void *dest, const void * src, rt_base_t len);
char * config_16_copy(void *dest, const void * src, rt_base_t len);
char * config_32_copy(void *dest, const void * src, rt_base_t len);
char * config_64_copy(void *dest, const void * src, rt_base_t len);

void copy_section
(
    rt_uint64_t * p_load,
    rt_uint64_t * p_vma,
    rt_uint64_t * p_vma_end
);
void zero_section
(
    rt_uint64_t *__sbss_start,
    rt_uint64_t * __sbss_end
);
void load_virtual_rom(void);

void count_section
(
    rt_uint64_t * start_address,
    rt_uint64_t * end_address,
    rt_uint64_t * start_value
);

#ifdef __cplusplus
}
#endif


#endif
