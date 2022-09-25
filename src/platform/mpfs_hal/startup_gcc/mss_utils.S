/*******************************************************************************
 * Copyright 2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
#include "cpuport.h"
/***************************************************************************
 * @file mss_utils.S
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief utilities used by mpfs-hal startup code
 *
 */
  .section .text.init,"ax", %progbits
  .align 3

/***********************************************************************************
 *
 * pdma_transfer
 * Only used by the mpfs hal. App code uses the provided driver.
 *
 *   a0 = dest
 *   a1 = src
 *   a2 = length
 *   a3 = PDMA Base Address - 0x3000000 + (0x01000 * PDMA_CHANNEL)
 */
    .globl  pdma_transfer
    .type   pdma_transfer, @function
pdma_transfer:
    mv  t1,a0
    mv t0, a3           // Base address
    li t1, 1
    sw t1, 0(t0)        // claim
    li t1, 0
    sw t1, 4(t0)        // read[31:28]/write[27:24] size 0=>1byte, 1 =>2 bytes etx
    mv t1, a2           // SIZE
    sd t1, 8(t0)        // bytes
    mv t1, a0           // dest address
    sd t1, 16(t0)       // dest
    mv t1, a1           // source address
    sd t1, 24(t0)       // src
    li t1, 0xff000000
    sw t1, 4(t0)        // full speed copy
    li t1, 3
    sw t1, 0(t0)        // start transfer
    fence
    ret

/***********************************************************************************
 *
 * pdma_transfer_complete
 * Loops until transfer complete
 * Only used by the mpfs hal. App code uses the provided driver.
 *
 *    a0 = PDMA Base Address - 0x3000000 + (0x01000 * PDMA_CHANNEL)
 */
    //
    .globl  pdma_transfer_complete
    .type   pdma_transfer_complete, @function
pdma_transfer_complete:
    mv t0, a0           // Base address
1: // wait for completion
    lw t1, 0(t0)
    andi t1, t1, 2
    bnez t1, 1b
    // release DMA
    sw zero, 0(t0)
    ret

 /***********************************************************************************
 *
 *  rt_thread self defined functions.
 */

/*
 * int exclusive_read_write(int* lock, int new_num);
 */
    .global exclusive_read_write
    .type  exclusive_read_write, @function
exclusive_read_write:
retry:
    lr.d a3, (a0)
    bnez a3, retry
    sc.d a3, a1, (a0)
    bnez a3, retry

/*
 * #ifdef RT_USING_SMP
 * void rt_hw_context_switch_to(rt_ubase_t to, stuct rt_thread *to_thread);
 * #else
 * void rt_hw_context_switch_to(rt_ubase_t to);
 * #endif
 * a0 --> to
 * a1 --> to_thread
 */
    .globl rt_hw_context_switch_to
    .type  rt_hw_context_switch_to, @function
rt_hw_context_switch_to:
    LOAD sp, (a0)

#ifdef RT_USING_SMP
    mv   a0,   a1
    call  rt_cpus_lock_status_restore
#endif
    LOAD a0,   2 * REGBYTES(sp)
    csrw mstatus, a0
    j    rt_hw_context_switch_exit

/*
 * #ifdef RT_USING_SMP
 * void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread);
 * #else
 * void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
 * #endif
 *
 * a0 --> from
 * a1 --> to
 * a2 --> to_thread
 */
    .globl rt_hw_context_switch
    .type  rt_hw_context_switch, @function
rt_hw_context_switch:
    /* saved from thread context
     *     x1/ra       -> sp(0)
     *     x1/ra       -> sp(1)
     *     mstatus.mie -> sp(2)
     *     x(i)        -> sp(i-4)
     */
#ifdef ARCH_RISCV_FPU
    addi    sp, sp, -32 * FREGBYTES

    FSTORE  f0, 0 * FREGBYTES(sp)
    FSTORE  f1, 1 * FREGBYTES(sp)
    FSTORE  f2, 2 * FREGBYTES(sp)
    FSTORE  f3, 3 * FREGBYTES(sp)
    FSTORE  f4, 4 * FREGBYTES(sp)
    FSTORE  f5, 5 * FREGBYTES(sp)
    FSTORE  f6, 6 * FREGBYTES(sp)
    FSTORE  f7, 7 * FREGBYTES(sp)
    FSTORE  f8, 8 * FREGBYTES(sp)
    FSTORE  f9, 9 * FREGBYTES(sp)
    FSTORE  f10, 10 * FREGBYTES(sp)
    FSTORE  f11, 11 * FREGBYTES(sp)
    FSTORE  f12, 12 * FREGBYTES(sp)
    FSTORE  f13, 13 * FREGBYTES(sp)
    FSTORE  f14, 14 * FREGBYTES(sp)
    FSTORE  f15, 15 * FREGBYTES(sp)
    FSTORE  f16, 16 * FREGBYTES(sp)
    FSTORE  f17, 17 * FREGBYTES(sp)
    FSTORE  f18, 18 * FREGBYTES(sp)
    FSTORE  f19, 19 * FREGBYTES(sp)
    FSTORE  f20, 20 * FREGBYTES(sp)
    FSTORE  f21, 21 * FREGBYTES(sp)
    FSTORE  f22, 22 * FREGBYTES(sp)
    FSTORE  f23, 23 * FREGBYTES(sp)
    FSTORE  f24, 24 * FREGBYTES(sp)
    FSTORE  f25, 25 * FREGBYTES(sp)
    FSTORE  f26, 26 * FREGBYTES(sp)
    FSTORE  f27, 27 * FREGBYTES(sp)
    FSTORE  f28, 28 * FREGBYTES(sp)
    FSTORE  f29, 29 * FREGBYTES(sp)
    FSTORE  f30, 30 * FREGBYTES(sp)
    FSTORE  f31, 31 * FREGBYTES(sp)

#endif
    addi  sp,  sp, -32 * REGBYTES
    STORE sp,  (a0)

    STORE x1,   0 * REGBYTES(sp)
    STORE x1,   1 * REGBYTES(sp)

    csrr a0, mstatus
    andi a0, a0, 8
    beqz a0, save_mpie
    li   a0, 0x80
save_mpie:
    STORE a0,   2 * REGBYTES(sp)

    STORE x4,   4 * REGBYTES(sp)
    STORE x5,   5 * REGBYTES(sp)
    STORE x6,   6 * REGBYTES(sp)
    STORE x7,   7 * REGBYTES(sp)
    STORE x8,   8 * REGBYTES(sp)
    STORE x9,   9 * REGBYTES(sp)
    STORE x10, 10 * REGBYTES(sp)
    STORE x11, 11 * REGBYTES(sp)
    STORE x12, 12 * REGBYTES(sp)
    STORE x13, 13 * REGBYTES(sp)
    STORE x14, 14 * REGBYTES(sp)
    STORE x15, 15 * REGBYTES(sp)
    STORE x16, 16 * REGBYTES(sp)
    STORE x17, 17 * REGBYTES(sp)
    STORE x18, 18 * REGBYTES(sp)
    STORE x19, 19 * REGBYTES(sp)
    STORE x20, 20 * REGBYTES(sp)
    STORE x21, 21 * REGBYTES(sp)
    STORE x22, 22 * REGBYTES(sp)
    STORE x23, 23 * REGBYTES(sp)
    STORE x24, 24 * REGBYTES(sp)
    STORE x25, 25 * REGBYTES(sp)
    STORE x26, 26 * REGBYTES(sp)
    STORE x27, 27 * REGBYTES(sp)
    STORE x28, 28 * REGBYTES(sp)
    STORE x29, 29 * REGBYTES(sp)
    STORE x30, 30 * REGBYTES(sp)
    STORE x31, 31 * REGBYTES(sp)

    /* restore to thread context
     * sp(0) -> epc;
     * sp(1) -> ra;
     * sp(i) -> x(i+2)
     */
    LOAD sp,  (a1)

#ifdef RT_USING_SMP
    mv   a0,   a2
    call  rt_cpus_lock_status_restore
#endif /*RT_USING_SMP*/

    j rt_hw_context_switch_exit

#ifdef RT_USING_SMP
/*
 * void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread);
 *
 * a0 --> context
 * a1 --> from
 * a2 --> to
 * a3 --> to_thread
 */
    .globl rt_hw_context_switch_interrupt
    .type  rt_hw_context_switch_interrupt, @function
rt_hw_context_switch_interrupt:

    STORE a0, 0(a1)

    LOAD  sp, 0(a2)
    move  a0, a3
    call rt_cpus_lock_status_restore
    j rt_hw_context_switch_exit

#endif

/** */
    .globl rt_hw_context_switch_exit
    .type  rt_hw_context_switch_exit, @function
rt_hw_context_switch_exit:
    /* resw ra to mepc */
    LOAD a0,   0 * REGBYTES(sp)
    csrw mepc, a0

    LOAD x1,   1 * REGBYTES(sp)

    li    t0, 0x00007800
    csrw  mstatus, t0
    LOAD a0,   2 * REGBYTES(sp)
    csrs mstatus, a0

    LOAD x4,   4 * REGBYTES(sp)
    LOAD x5,   5 * REGBYTES(sp)
    LOAD x6,   6 * REGBYTES(sp)
    LOAD x7,   7 * REGBYTES(sp)
    LOAD x8,   8 * REGBYTES(sp)
    LOAD x9,   9 * REGBYTES(sp)
    LOAD x10, 10 * REGBYTES(sp)
    LOAD x11, 11 * REGBYTES(sp)
    LOAD x12, 12 * REGBYTES(sp)
    LOAD x13, 13 * REGBYTES(sp)
    LOAD x14, 14 * REGBYTES(sp)
    LOAD x15, 15 * REGBYTES(sp)
    LOAD x16, 16 * REGBYTES(sp)
    LOAD x17, 17 * REGBYTES(sp)
    LOAD x18, 18 * REGBYTES(sp)
    LOAD x19, 19 * REGBYTES(sp)
    LOAD x20, 20 * REGBYTES(sp)
    LOAD x21, 21 * REGBYTES(sp)
    LOAD x22, 22 * REGBYTES(sp)
    LOAD x23, 23 * REGBYTES(sp)
    LOAD x24, 24 * REGBYTES(sp)
    LOAD x25, 25 * REGBYTES(sp)
    LOAD x26, 26 * REGBYTES(sp)
    LOAD x27, 27 * REGBYTES(sp)
    LOAD x28, 28 * REGBYTES(sp)
    LOAD x29, 29 * REGBYTES(sp)
    LOAD x30, 30 * REGBYTES(sp)
    LOAD x31, 31 * REGBYTES(sp)

    addi sp,  sp, 32 * REGBYTES

#ifdef ARCH_RISCV_FPU
    FLOAD   f0, 0 * FREGBYTES(sp)
    FLOAD   f1, 1 * FREGBYTES(sp)
    FLOAD   f2, 2 * FREGBYTES(sp)
    FLOAD   f3, 3 * FREGBYTES(sp)
    FLOAD   f4, 4 * FREGBYTES(sp)
    FLOAD   f5, 5 * FREGBYTES(sp)
    FLOAD   f6, 6 * FREGBYTES(sp)
    FLOAD   f7, 7 * FREGBYTES(sp)
    FLOAD   f8, 8 * FREGBYTES(sp)
    FLOAD   f9, 9 * FREGBYTES(sp)
    FLOAD   f10, 10 * FREGBYTES(sp)
    FLOAD   f11, 11 * FREGBYTES(sp)
    FLOAD   f12, 12 * FREGBYTES(sp)
    FLOAD   f13, 13 * FREGBYTES(sp)
    FLOAD   f14, 14 * FREGBYTES(sp)
    FLOAD   f15, 15 * FREGBYTES(sp)
    FLOAD   f16, 16 * FREGBYTES(sp)
    FLOAD   f17, 17 * FREGBYTES(sp)
    FLOAD   f18, 18 * FREGBYTES(sp)
    FLOAD   f19, 19 * FREGBYTES(sp)
    FLOAD   f20, 20 * FREGBYTES(sp)
    FLOAD   f21, 21 * FREGBYTES(sp)
    FLOAD   f22, 22 * FREGBYTES(sp)
    FLOAD   f23, 23 * FREGBYTES(sp)
    FLOAD   f24, 24 * FREGBYTES(sp)
    FLOAD   f25, 25 * FREGBYTES(sp)
    FLOAD   f26, 26 * FREGBYTES(sp)
    FLOAD   f27, 27 * FREGBYTES(sp)
    FLOAD   f28, 28 * FREGBYTES(sp)
    FLOAD   f29, 29 * FREGBYTES(sp)
    FLOAD   f30, 30 * FREGBYTES(sp)
    FLOAD   f31, 31 * FREGBYTES(sp)

    addi    sp, sp, 32 * FREGBYTES
#endif

    mret

 /***********************************************************************************
 *
 * memfill() - fills memory, alternate to lib function when not available
 */
    // memfill helper function:
    //  a0 = dest
    //  a1 = value to fill
    //  a2 = length
    .globl  memfill
    .type   memfill, @function
memfill:
    mv  t1,a0
    mv  t2,a1
    beqz    a2,2f
1:
    sb  t2,0(t1)
    addi    a2,a2,-1
    addi    t1,t1,1
    bnez    a2,1b
2:
    ret

/***********************************************************************************
 *
 * The following config_copy() symbol overrides the weak symbol in the HAL and does
 * a safe copy of HW config data
 */
    // config_copy helper function:
    //  a0 = dest
    //  a1 = src
    //  a2 = length
    .globl  config_copy
    .type   config_copy, @function
config_copy:
    mv  t1,a0
    beqz    a2,2f
1:
    lb  t2,0(a1)
    sb  t2,0(t1)
    addi    a2,a2,-1
    addi    t1,t1,1
    addi    a1,a1,1
    bnez    a2,1b
2:
    ret

 /***********************************************************************************
 *
 * config_16_copy () Copies a word at a time, used when copying to contigous registers
 */
    // config_16_copy helper function:
    //  a0 = dest
    //  a1 = src
    //  a2 = length
    .globl  config_16_copy
    .type   config_16_copy, @function
config_16_copy:
    mv  t1,a0
    beqz    a2,2f
1:
    lh  t2,0(a1)
    sh  t2,0(t1)
    addi    a2,a2,-2
    addi    t1,t1,2
    addi    a1,a1,2
    bnez    a2,1b
2:
    ret

/***********************************************************************************
 *
 * config_32_copy () Copies a word at a time, used when copying to contigous registers
 */
    // config_copy helper function:
    //  a0 = dest
    //  a1 = src
    //  a2 = length
    .globl  config_32_copy
    .type   config_32_copy, @function
config_32_copy:
    mv  t1,a0
    beqz    a2,2f
1:
    lw  t2,0(a1)
    sw  t2,0(t1)
    addi    a2,a2,-4
    addi    t1,t1,4
    addi    a1,a1,4
    bnez    a2,1b
2:
    ret

 /***********************************************************************************
 *
 * config_64_copy - copying using 64 bit loads, addresses must be on 64 bit boundary
 */
    // config_64_copy helper function:
    //  a0 = dest
    //  a1 = src
    //  a2 = length
    .globl  config_64_copy
    .type   config_64_copy, @function
config_64_copy:
    mv  t1,a0
    beqz    a2,2f
1:
    ld  t2,0(a1)
    sd  t2,0(t1)
    addi    a2,a2,-8
    addi    t1,t1,8
    addi    a1,a1,8
    bnez    a2,1b
2:
    ret
