/*
 * linux/arch/arm/mach-omap2/sleep_34xx.S
 *
 * (C) Copyright 2004-2008
 * Texas Instruments
 * Karthik Dasu <karthik-dp@ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <mach/io.h>
#include <mach/pm.h>
#include "ti-compat.h"

#define PM_PREPWSTST_CORE_V	IO_ADDRESS(PRM_BASE + 0xAE8)
#define PM_PREPWSTST_CORE_P	(PRM_BASE + 0xAE8)
#define PM_PREPWSTST_MPU_V	IO_ADDRESS(PRM_BASE + 0x9E8)
#define PM_PWSTCTRL_MPU_P	(PRM_BASE + 0x9E0)
#define SCRATCHPAD_BASE_P	(OMAP343X_SCM_BASE + 0x910)
#define SRAM_BASE_P		0x40200000
#define CONTROL_STAT_P		(OMAP343X_SCM_BASE + 0x2F0)
#define	CM_ICLKEN1_CORE_V	IO_ADDRESS(CM_BASE + 0xA10)
#define CM_IDLEST1_CORE_V	IO_ADDRESS(CM_BASE + 0xA20)
#define	SDRC_DLLA_CTRL_V	IO_ADDRESS(OMAP343X_SDRC_BASE + 0x060)
#define	SDRC_DLLA_STATUS_V	IO_ADDRESS(OMAP343X_SDRC_BASE + 0x064)
#define SDRC_POWER_V		IO_ADDRESS(SDRC_BASE + 0x070)
#define SDRC_SYSCONFIG_P        (SDRC_BASE + 0x010)
#define SDRC_MR_P_0             (SDRC_BASE + 0x84)
#define SDRC_EMR2_P_0           (SDRC_BASE + 0x8c)
#define SDRC_MANUAL_P_0         (SDRC_BASE + 0xa8)
#define SDRC_MR_P_1             (SDRC_BASE + 0x84 + 0x30)
#define SDRC_EMR2_P_1           (SDRC_BASE + 0x8c + 0x30)
#define SDRC_MANUAL_P_1         (SDRC_BASE + 0xa8 + 0x30)
#define SDRC_SCRATCHPAD_SEM	0xd800291C
#define CORTEX_CORSIGHT_OFF     (0x00011000)
#define CORTEX_EMU_DEBUG_V      (L4_EMU_34XX_VIRT + CORTEX_CORSIGHT_OFF)
#define CORTEX_EMU_DEBUG_P      (L4_EMU_34XX_PHYS + CORTEX_CORSIGHT_OFF)
#define ETM_CORSIGHT_OFF        (0x00010000)
#define CORTEX_EMU_ETM_V        (L4_EMU_34XX_VIRT + ETM_CORSIGHT_OFF)
#define CORTEX_EMU_ETM_P        (L4_EMU_34XX_PHYS + ETM_CORSIGHT_OFF)

	.text
/* Function to aquire the semaphore in scratchpad */
ENTRY(lock_scratchpad_sem)
	stmfd   sp!, {lr}     @ save registers on stack
wait_sem:
	mov	r0,#1
	ldr     r1, sdrc_scratchpad_sem
wait_loop:
	ldr	r2, [r1]		@ load the lock value
	cmp	r2, r0		@ is the lock free ?
	beq     wait_loop	@ not free...
	swp	r2, r0, [r1]	@ semaphore free so lock it and proceed
	cmp	r2, r0		@ did we succeed ?
	beq	wait_sem	@ no - try again
	ldmfd   sp!, {pc}     @ restore regs and return
sdrc_scratchpad_sem:
	.word SDRC_SCRATCHPAD_SEM
ENTRY(lock_scratchpad_sem_sz)
	.word   . - lock_scratchpad_sem

	.text
/* Function to release the scratchpad semaphore */
ENTRY(unlock_scratchpad_sem)
	stmfd   sp!, {lr}	@ save registers on stack
	ldr	r3, sdrc_scratchpad_sem
	mov	r2,#0
	str	r2,[r3]
	ldmfd   sp!, {pc}	@ restore regs and return
ENTRY(unlock_scratchpad_sem_sz)
	.word   . - unlock_scratchpad_sem

	.text
/* Function call to get the restore pointer for resume from OFF */
ENTRY(get_restore_pointer)
        stmfd   sp!, {lr}     @ save registers on stack
	adr	r0, restore
        ldmfd   sp!, {pc}     @ restore regs and return
ENTRY(get_restore_pointer_sz)
        .word   . - get_restore_pointer

	.text
/* Function call to get the restore pointer for for ES3 to resume from OFF */
ENTRY(get_es3_restore_pointer)
        stmfd   sp!, {lr}     @ save registers on stack
	adr	r0, restore_es3
        ldmfd   sp!, {pc}     @ restore regs and return
ENTRY(get_es3_restore_pointer_sz)
        .word   . - get_es3_restore_pointer

ENTRY(es3_sdrc_fix)
        ldr     r4, sdrc_syscfg     @ get config addr
        ldr     r5, [r4]            @ get value
        tst     r5, #0x100          @ is part access blocked
        it      eq
        biceq   r5, r5, #0x100      @ clear bit if set
        str     r5, [r4]            @ write back change
        ldr     r4, sdrc_mr_0       @ get config addr
        ldr     r5, [r4]            @ get value
        str     r5, [r4]            @ write back change
        ldr     r4, sdrc_emr2_0     @ get config addr
        ldr     r5, [r4]            @ get value
        str     r5, [r4]            @ write back change
	ldr     r4, sdrc_manual_0   @ get config addr
	mov     r5, #0x2            @ autorefresh command
	str     r5, [r4]            @ kick off refreshes
        ldr     r4, sdrc_mr_1       @ get config addr
        ldr     r5, [r4]            @ get value
        str     r5, [r4]            @ write back change
        ldr     r4, sdrc_emr2_1     @ get config addr
        ldr     r5, [r4]            @ get value
        str     r5, [r4]            @ write back change
        ldr     r4, sdrc_manual_1   @ get config addr
        mov     r5, #0x2            @ autorefresh command
        str     r5, [r4]            @ kick off refreshes
	bx	lr
sdrc_syscfg:
        .word SDRC_SYSCONFIG_P
sdrc_mr_0:
        .word SDRC_MR_P_0
sdrc_emr2_0:
	.word SDRC_EMR2_P_0
sdrc_manual_0:
        .word SDRC_MANUAL_P_0
sdrc_mr_1:
	.word SDRC_MR_P_1
sdrc_emr2_1:
	.word SDRC_EMR2_P_1
sdrc_manual_1:
	.word SDRC_MANUAL_P_1
ENTRY(es3_sdrc_fix_sz)
	.word	. - es3_sdrc_fix

/* Function to call rom code to save secure ram context */
ENTRY(save_secure_ram_context)
	stmfd	sp!, {r1-r12, lr}		@ save registers on stack
api_params:
	.word   0x4, 0x0, 0x0, 0x1, 0x1
	/* b api_params*/	@ enable to debug save code
	adr     r3, api_params		@ r3 points to parameters
	str	r0, [r3,#0x4]		@ r0 has sdram address
	ldr     r12, high_mask
	and     r3, r3, r12
	ldr     r12, sram_phy_addr_mask
	orr     r3, r3, r12
	mov     r0, #25			@ set service ID for PPA
	mov     r12, r0			@ copy secure Service ID in r12
	mov     r1, #0			@ set task id for ROM code in r1
	mov     r2, #4			@ set some flags in r2, r6
	mov     r6, #0xff
	mcr     p15, 0, r0, c7, c5, 4	@ data write barrier
	mcr     p15, 0, r0, c7, c10, 5	@ data memory barrier
	.word   0xE1600071		@ call SMI monitor (smi #1)
	nop
	nop
	nop
	nop
	ldmfd	sp!, {r1-r12, pc}
sram_phy_addr_mask:
        .word   0x40200000
high_mask:
        .word   0xffff
ENTRY(save_secure_ram_context_sz)
        .word   . - save_secure_ram_context

/* Function to call rom code to generate a new Random Number */
ENTRY(start_new_rng)
	stmfd   sp!, {r1-r12, lr}       @ save registers on stack
api_params_rng:
	.word   0x1, 0x0        @ Pass one argument with dummy value
	adr     r3, api_params_rng      @ r3 points to parameters
	ldr     r12, high_mask_rng
	and     r3, r3, r12
	ldr     r12, sram_phy_addr_mask_rng
	orr     r3, r3, r12
	mov     r0, #50                 @ set service ID for PPA
	mov     r12, r0                 @ copy secure Service ID in r12
	mov     r1, #0                  @ set task id for ROM code in r1
	mov     r2, #6                  @ set some flags in r2, r6
	mov     r6, #0xff
	mcr     p15, 0, r0, c7, c5, 4   @ data write barrier
	mcr     p15, 0, r0, c7, c10, 5  @ data memory barrier
	.word   0xE1600071              @ call SMI monitor (smi #1)
	b finished_RNG
	nop             @ ROM code jumps, after intr handling (LR+0xR+0x4)
	.word   0xF1080080      @ SMI call for enabling interrupts
	mov     r12, #0xFE      @ Flag (return from intr handling) for our SMI
	.word     0xE1600071    @ Again call our SMI
finished_RNG:
	nop
	nop
	nop
	nop
	ldmfd   sp!, {r1-r12, pc}
sram_phy_addr_mask_rng:
	.word   0x40200000
high_mask_rng:
	.word   0xffff
ENTRY(start_new_rng_sz)
	.word   . - start_new_rng

/*
 * Forces OMAP into idle state
 *
 * omap34xx_cpu_suspend() - This bit of code just executes the WFI
 * for normal idles.
 *
 * Note: This code get's copied to internal SRAM at boot. When the OMAP
 *	 wakes up it continues execution at the point it went to sleep.
 */
ENTRY(omap34xx_cpu_suspend)
	stmfd	sp!, {r0-r12, lr}		@ save registers on stack
loop:
	/*b	loop*/	@Enable to debug by stepping through code
	/* r0 contains restore pointer in sdram */
	/* r1 contains information about saving context */
	ldr     r4, sdrc_power          @ read the SDRC_POWER register
	ldr     r5, [r4]                @ read the contents of SDRC_POWER
	orr     r5, r5, #0x40           @ enable self refresh on idle req
	str     r5, [r4]                @ write back to SDRC_POWER register

	cmp	r1, #0x0
	/* If context save is required, do that and execute wfi */
	bne	save_context_wfi
	/* Data memory barrier and Data sync barrier */
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4
	mcr	p15, 0, r1, c7, c10, 5

	wfi				@ wait for interrupt

	nop
	nop
        nop
	bl wait_sdrc_ok

	ldmfd	sp!, {r0-r12, pc}		@ restore regs and return
restore_es3:
	/*b restore_es3*/ 		@ Enable to debug restore code
	ldr	r5, pm_prepwstst_core_p
	ldr	r4, [r5]
	and     r4, r4, #0x3
	cmp     r4, #0x0        @ Check if previous power state of CORE is OFF
	bne	restore
	bl	es3_sdrc_fix
restore:
	/*b restore*/	@ Enable to debug restore code
        /* Check what was the reason for mpu reset and store the reason in r9*/
        /* 1 - Only L1 and logic lost */

	ldr     r1, pm_pwstctrl_mpu
	ldr	r2, [r1]
	and     r2, r2, #0x3
	cmp     r2, #0x0	@ Check if target power state was OFF or RET
	ite	eq
        moveq   r9, #0x3        @ MPU OFF => L1 and L2 lost
	beq     restore_from_off
	cmp     r2, #0x1
	ite     eq
	moveq   r9, #0x3
	movne	r9, #0x1	@ Only L1 and L2 lost => avoid L2 invalidation
	bne	logic_l1_restore
restore_from_off:
	ldr	r0, control_stat
	ldr	r1, [r0]
	and	r1, #0x700
	cmp	r1, #0x300
	beq	l2_inv_gp
l2_inv_api_params:
	.word   0x1, 0x0
	mov     r0, #40		@ set service ID for PPA
	mov     r12, r0		@ copy secure Service ID in r12
	mov     r1, #0		@ set task id for ROM code in r1
	mov     r2, #4		@ set some flags in r2, r6
	mov     r6, #0xff
	adr     r3, l2_inv_api_params	@ r3 points to dummy parameters
	mcr     p15, 0, r0, c7, c5, 4	@ data write barrier
	mcr     p15, 0, r0, c7, c10, 5	@ data memory barrier
	.word   0xE1600071		@ call SMI monitor (smi #1)
	/* Write to Aux control register to set some bits */
write_aux_control_params:
	.word   0x1, 0x72
	mov     r0, #42		@ set service ID for PPA
	mov     r12, r0		@ copy secure Service ID in r12
	mov     r1, #0		@ set task id for ROM code in r1
	mov     r2, #4		@ set some flags in r2, r6
	mov     r6, #0xff
	adr     r3, write_aux_control_params	@ r3 points to parameters
	mcr     p15, 0, r0, c7, c5, 4	@ data write barrier
	mcr     p15, 0, r0, c7, c10, 5	@ data memory barrier
	.word   0xE1600071		@ call SMI monitor (smi #1)

	ldr	r0, control_stat
	ldr	r1, [r0]
	and	r1, #0x700
	cmp	r1, #0x300
	bne	logic_l1_restore
l2_inv_gp:
	/* Execute smi to invalidate L2 cache */
	mov r12, #0x1                         @ set up to invalide L2
smi:    .word 0xE1600070                @ Call SMI monitor (smieq)
	/* Write to Aux control register to set some bits */
	mov	r0, #0x72
	mov	r12, #0x3
	.word 0xE1600070                @ Call SMI monitor (smieq)
logic_l1_restore:
	mov	r1, #0
	/* Invalidate all instruction caches to PoU
	 * and flush branch target cache */
	mcr	p15, 0, r1, c7, c5, 0

	ldr	r4, scratchpad_base
	ldr	r3, [r4,#0xBC]

	ldmia	r3!, {r4-r6}
	mov	sp, r4
	msr	spsr_cxsf, r5
	mov	lr, r6

	ldmia	r3!, {r4-r9}
	/* Coprocessor access Control Register */
	mcr p15, 0, r4, c1, c0, 2

	/* TTBR0 */
	MCR p15, 0, r5, c2, c0, 0
	/* TTBR1 */
	MCR p15, 0, r6, c2, c0, 1
	/* Translation table base control register */
	MCR p15, 0, r7, c2, c0, 2
	/*domain access Control Register */
	MCR p15, 0, r8, c3, c0, 0
	/* data fault status Register */
	MCR p15, 0, r9, c5, c0, 0

	ldmia  r3!,{r4-r8}
	/* instruction fault status Register */
	MCR p15, 0, r4, c5, c0, 1
	/*Data Auxiliary Fault Status Register */
	MCR p15, 0, r5, c5, c1, 0
	/*Instruction Auxiliary Fault Status Register*/
	MCR p15, 0, r6, c5, c1, 1
	/*Data Fault Address Register */
	MCR p15, 0, r7, c6, c0, 0
	/*Instruction Fault Address Register*/
	MCR p15, 0, r8, c6, c0, 2

	/* Performance Counters */
	/* Counter Specific Info */

	mov 	r4, #0x00
perf_counters_restore:
	/*Counter 0-3*/
	MCR	p15, 0, r4, c9, c12, 5
	ldmia  r3!,{r5-r6}
	/* Event Selection Register for Counter 0-3*/
	MCR 	p15, 0, r5, c9, c13, 1
	/* Performance Monitor Count Register for Counter 0-3*/
	MCR	p15, 0, r6, c9, c13, 2
	add	r4, r4, #0x1
	cmp	r4, #0x4
	bne	perf_counters_restore

	ldmia  r3!,{r4-r8}
	/* Cycle Count Register */
	MCR	p15, 0, r4, c9, c13, 0
	/* Overflow Flag Status Register */
	MCR 	p15, 0, r5, c9, c12, 3
	/* Interrupt Enable Set Register */
	MCR	p15, 0, r6, c9, c14, 1
	/* Count Enable Set Register */
	MCR 	p15, 0, r7, c9, c12, 1
	/* Performance Monitor Control Register */
	MCR	p15, 0, r8, c9, c12, 0
	ldmia  r3!,{r4-r7}

	/* user r/w thread and process ID */
	MCR p15, 0, r4, c13, c0, 2
	/* user ro thread and process ID */
	MCR p15, 0, r5, c13, c0, 3
	/*Privileged only thread and process ID */
	MCR p15, 0, r6, c13, c0, 4
	/* cache size selection */
	MCR p15, 2, r7, c0, c0, 0
	ldmia  r3!,{r4-r8}
	/* Data TLB lockdown registers */
	MCR p15, 0, r4, c10, c0, 0
	/* Instruction TLB lockdown registers */
	MCR p15, 0, r5, c10, c0, 1
	/* Secure or Nonsecure Vector Base Address */
	MCR p15, 0, r6, c12, c0, 0
	/* FCSE PID */
	MCR p15, 0, r7, c13, c0, 0
	/* Context PID */
	MCR p15, 0, r8, c13, c0, 1

	ldmia  r3!,{r4-r5}
	/* primary memory remap register */
	MCR p15, 0, r4, c10, c2, 0
	/*normal memory remap register */
	MCR p15, 0, r5, c10, c2, 1

#ifdef CONFIG_ENABLE_OFF_MODE_JTAG_ETM_DEBUG
	/*
	 * Restore Coresight debug registers
	 */
	ldr     r6, debug_pbase      /* base paddr of CortexA8-Debug */
	ldr     r4, debug_xlar_key   /* get lock key for OSLAR */
	bl      unlock_debug         /* remove global lock if set */
	str     r4, [r6, #0x300]     /* reset-pointer (already locked) */
	ldr     r4, [r6, #0x308]     /* dummy read */
	ldr     r4, [r3], #4         /* load save size */
	cmp     r4, #0               /* check for zero */
debug_restore:
	ittt    ne                   /* t2/compat if-then block */
	ldrne   r5, [r3], #4         /* get save value */
	strne   r5, [r6,#0x308]      /* restore cp14 value */
	subnes  r4, r4, #1           /* decrement loop */
	bne     debug_restore        /* loop till done */
	str     r5, [r6, #0x300]     /* clear lock */
	/*
	 * Restore CoreSight ETM registers
	 */
	ldr     r6, etm_pbase        /* base paddr of ETM */
	ldr     r4, debug_xlar_key   /* get lock key for OSLAR */
	bl      unlock_debug         /* remove global lock if set */
	ldr     r4, [r3], #4         /* load save size */
	cmp     r4, #0               /* check for zero */
	beq     etm_skip
	sub     r4, #1
	ldr     r7, [r3], #4         /* get/store first value to r7 */
	mov     r5, #0x3             /* enable programming in ETMCR*/
	lsl     r5, r5, #10
	orr     r5, r7
	str     r5,[r6], #4
	cmp     r4, #0               /* check for zero */
etm_restore:
	ldrne   r5, [r3], #4         /* get save value */
	strne   r5, [r6], #4         /* restore cp14 value */
	subnes  r4, r4, #1           /* decrement loop */
	bne     etm_restore          /* loop till done */
	ldr     r6, etm_pbase        /* base paddr of ETM */
	str     r7,[r6]              /* dis-able programming in ETMCR */
etm_skip:
#endif

	/* Restore registers for other modes from SDRAM */
	/* Save current mode */
	mrs	r7, cpsr

	/* FIQ mode */
	bic	r0, r7, #0x1F
	orr	r0, r0, #0x11
	msr	cpsr, r0
	ldmia	r3!, {r8-r12}
	/* load the SP and LR from SDRAM */
	ldmia  r3!,{r4-r6}
	mov    sp, r4	/*update the SP */
	mov    lr, r5	/*update the LR */
	msr    spsr, r6	/*update the SPSR*/

	/* IRQ mode */
	bic    r0, r7, #0x1F
	orr    r0, r0, #0x12
	msr    cpsr, r0	/*go into IRQ mode*/
	ldmia  r3!,{r4-r6}	/*load the SP and LR from SDRAM*/
	mov    sp, r4	/*update the SP */
	mov    lr, r5	/*update the LR */
	msr    spsr, r6	/*update the SPSR */

	/* ABORT mode */
	bic    r0, r7, #0x1F
	orr    r0, r0, #0x17
	msr    cpsr, r0	/* go into ABORT mode */
	ldmia  r3!,{r4-r6}	/*load the SP and LR from SDRAM */
	mov    sp, r4		/*update the SP */
	mov    lr, r5		/*update the LR */
	msr    spsr, r6		/*update the SPSR */

	/* UNDEEF mode */
	bic    r0, r7, #0x1F
	orr    r0, r0, #0x1B
	msr    cpsr, r0		/*go into UNDEF mode */
	ldmia  r3!,{r4-r6}	/*load the SP and LR from SDRAM */
	mov    sp, r4		/*update the SP*/
	mov    lr, r5		/*update the LR*/
	msr    spsr, r6		/*update the SPSR*/

	/* SYSTEM (USER) mode */
	bic    r0, r7, #0x1F
	orr    r0, r0, #0x1F
	msr    cpsr, r0		/*go into USR mode */
	ldmia  r3!,{r4-r6}	/*load the SP and LR from SDRAM*/
	mov    sp, r4		/*update the SP */
	mov    lr, r5		/*update the LR */
	msr    spsr, r6		/*update the SPSR */
	msr    cpsr, r7		/*back to original mode*/

	/* Restore cpsr */
	ldmia	r3!,{r4}	/*load CPSR from SDRAM*/
	msr	cpsr, r4	/*store cpsr */

	/* Enabling MMU here */
	mrc	p15, 0, r7, c2, c0, 2 /* Read TTBRControl */
	/* Extract N (0:2) bits and decide whether to use TTBR0 or TTBR1*/
	and	r7, #0x7
	cmp	r7, #0x0
	beq	usettbr0
ttbr_error:
	/* More work needs to be done to support N[0:2] value other than 0
	* So looping here so that the error can be detected
	*/
	b	ttbr_error
usettbr0:
	mrc	p15, 0, r2, c2, c0, 0
	ldr	r5, ttbrbit_mask
	and	r2, r5
	mov	r4, pc
	ldr	r5, table_index_mask
	and	r4, r5 /* r4 = 31 to 20 bits of pc */
	/* Extract the value to be written to table entry */
	ldr	r1, table_entry
	add	r1, r1, r4 /* r1 has value to be written to table entry*/
	/* Getting the address of table entry to modify */
	lsr	r4, #18
	add	r2, r4 /* r2 has the location which needs to be modified */
	/* Storing previous entry of location being modified */
	ldr	r5, scratchpad_base
	ldr	r4, [r2]
	str	r4, [r5, #0xC0]
	/* Modify the table entry */
	str	r1, [r2]
	/* Storing address of entry being modified
	 * - will be restored after enabling MMU */
	ldr	r5, scratchpad_base
	str	r2, [r5, #0xC4]

	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 4	@ Flush prefetch buffer
	mcr	p15, 0, r0, c7, c5, 6	@ Invalidate branch predictor array
	mcr	p15, 0, r0, c8, c5, 0	@ Invalidate instruction TLB
	mcr	p15, 0, r0, c8, c6, 0	@ Invalidate data TLB
	/* Restore control register  but dont enable caches here*/
	/* Caches will be enabled after restoring MMU table entry */
	ldmia	r3!, {r4}
	/* Store previous value of control register in scratchpad */
	str	r4, [r5, #0xC8]
	ldr	r2, cache_pred_disable_mask
	and	r4, r2
	mcr	p15, 0, r4, c1, c0, 0

	ldmfd	sp!, {r0-r12, pc}		@ restore regs and return
save_context_wfi:
	/*b	save_context_wfi*/	@ enable to debug save code
	mov	r8, r0 /* Store SDRAM address in r8 */
        /* Check what that target sleep state is:stored in r1*/
        /* 1 - Only L1 and logic lost */
        /* 2 - Only L2 lost */
        /* 3 - Both L1 and L2 lost */
	cmp	r1, #0x2 /* Only L2 lost */
	beq	clean_l2
	cmp	r1, #0x1 /* L2 retained */
	ite	eq
	/* r9 stores whether to clean L2 or not*/
	moveq	r9, #0x0 /* Dont Clean L2 */
	movne	r9, #0x1 /* Clean L2 */
l1_logic_lost:
	/* Store sp and spsr to SDRAM */
	mov	r4, sp
	mrs	r5, spsr
	mov	r6, lr
	stmia	r8!, {r4-r6}
	/* Save all ARM registers */
	/* Coprocessor access control register */
	mrc	p15, 0, r6, c1, c0, 2
	stmia	r8!, {r6}
	/* TTBR0, TTBR1 and Translation table base control */
	mrc	p15, 0, r4, c2, c0, 0
	mrc	p15, 0, r5, c2, c0, 1
	mrc	p15, 0, r6, c2, c0, 2
	stmia	r8!, {r4-r6}
	/* Domain access control register, data fault status register,
	and instruction fault status register */
	mrc	p15, 0, r4, c3, c0, 0
	mrc	p15, 0, r5, c5, c0, 0
	mrc	p15, 0, r6, c5, c0, 1
	stmia	r8!, {r4-r6}
	/* Data aux fault status register, instruction aux fault status,
	datat fault address register and instruction fault address register*/
	mrc	p15, 0, r4, c5, c1, 0
	mrc	p15, 0, r5, c5, c1, 1
	mrc	p15, 0, r6, c6, c0, 0
	mrc	p15, 0, r7, c6, c0, 2
	stmia	r8!, {r4-r7}

	/* Performance Counters */
	/* Counter Specific Info */

	mov 	r4, #0x00
perf_counters_save:
	/*Counter 0-3*/
	mcr	p15, 0, r4, c9, c12, 5
	/* Event Selection Register for Counter 0-3*/
	mrc 	p15, 0, r5, c9, c13, 1
	/* Performance Monitor Count Register for Counter 0-3*/
	mrc	p15, 0, r6, c9, c13, 2
	stmia   r8!, {r5-r6}
	add	r4, r4, #0x1
	cmp	r4, #0x4
	bne	perf_counters_save

	/* Cycle Count Register */
	mrc	p15, 0, r4, c9, c13, 0
	/* Overflow Flag Status Register */
	mrc 	p15, 0, r5, c9, c12, 3
	/* Interrupt Enable Set Register */
	mrc	p15, 0, r6, c9, c14, 1
	/* Count Enable Set Register */
	mrc 	p15, 0, r7, c9, c12, 1
	stmia	r8!, {r4-r7}

	/* Performance Monitor Control Register */
	mrc	p15, 0, r4, c9, c12, 0
	stmia	r8!, {r4}
	/* user r/w thread and process ID, user r/o thread and process ID,
	priv only thread and process ID, cache size selection */
	mrc	p15, 0, r4, c13, c0, 2
	mrc	p15, 0, r5, c13, c0, 3
	mrc	p15, 0, r6, c13, c0, 4
	mrc	p15, 2, r7, c0, c0, 0
	stmia	r8!, {r4-r7}
	/* Data TLB lockdown, instruction TLB lockdown registers */
	mrc	p15, 0, r5, c10, c0, 0
	mrc	p15, 0, r6, c10, c0, 1
	stmia	r8!, {r5-r6}
	/* Secure or non secure vector base address, FCSE PID, Context PID*/
	mrc	p15, 0, r4, c12, c0, 0
	mrc	p15, 0, r5, c13, c0, 0
	mrc	p15, 0, r6, c13, c0, 1
	stmia	r8!, {r4-r6}
	/* Primary remap, normal remap registers */
	mrc	p15, 0, r4, c10, c2, 0
	mrc	p15, 0, r5, c10, c2, 1
	stmia	r8!, {r4-r5}
#ifdef CONFIG_ENABLE_OFF_MODE_JTAG_ETM_DEBUG
	/*
	 * Save Coresight debug registers
	 */
	ldr     r6, debug_vbase      /* base vaddr of CortexA8-Debug */
	ldr     r4, debug_xlar_key   /* get lock key for OSLAR */
	bl      unlock_debug         /* force global unlock */
	str     r4, [r6, #0x300]     /* lock debug access */
	ldr     r4, [r6, #0x308]     /* OSSRR returns size on first read */
	str     r4, [r8], #4         /* push item to save area */
	cmp     r4, #0               /* zero check */
debug_save:
	ittt    ne                   /* thumb 2 compat if-then block */
	ldrne   r5, [r6, #0x308]     /* get reg value */
	strne   r5, [r8], #4         /* push item to save area */
	subnes  r4, r4, #1           /* decrement size */
	bne     debug_save           /* loop till done */
	/*
	 * Save etm registers
	 */
	ldr     r6, etm_vbase        /* base vaddr of CortexA8-Debug */
	ldr     r4, debug_xlar_key   /* get lock key for OSLAR */
	bl      unlock_debug         /* force global unlock */
	mov     r4, #128             /* OSSRR returns size on first read */
	str     r4, [r8], #4         /* push item to save area */
	cmp     r4, #0               /* zero check */
etm_save:
	ldrne   r5, [r6], #4         /* get reg value */
	strne   r5, [r8], #4         /* push item to save area */
	subnes  r4, r4, #1           /* decrement size */
	bne     etm_save             /* loop till done */
#endif
	/* Store SP, LR, SPSR registers for SUP, FIQ, IRQ, ABORT and USER
	modes into SDRAM */

	/* move SDRAM address to r7 as r8 is banked in FIQ*/
	mov	r7, r8

	/* Save current mode */
	mrs	r2, cpsr
	/* FIQ mode */
	bic	r0, r2, #0x1F
	orr	r0, r0, #0x11
	msr	cpsr, r0 /* go to FIQ mode */
	stmia	r7!, {r8-r12}
	mov	r4, r13 /* move SP into r4*/
	mov	r5, r14
	mrs	r6, spsr
	stmia	r7!, {r4-r6}

	/* IRQ mode */
	bic	r0, r2, #0x1F
	orr	r0, r0, #0x12
	msr	cpsr, r0
	mov	r4, r13
	mov	r5, r14
	mrs	r6, spsr
	stmia	r7!, {r4-r6}

	/* Abort mode */
	bic	r0, r2, #0x1F
	orr	r0, r0, #0x17
	msr	cpsr, r0
	mov	r4, r13
	mov	r5, r14
	mrs	r6, spsr
	stmia	r7!, {r4-r6}

	/* UNDEF mode */
	bic	r0, r2, #0x1F
	orr	r0, r0, #0x1B
	msr	cpsr, r0
	mov	r4, r13
	mov	r5, r14
	mrs	r6, spsr
	stmia	r7!, {r4-r6}

	/* System (USER mode) */
	bic	r0, r2, #0x1F
	orr	r0, r0, #0x1F
	msr	cpsr, r0
	mov	r4, r13
	mov	r5, r14
	mrs	r6, spsr
	stmia	r7!, {r4-r6}

	/* Back to original mode */
	msr	cpsr, r2

	/* Store current cpsr*/
	stmia	r7!, {r2}

	mrc	p15, 0, r4, c1, c0, 0
	/* save control register */
	stmia	r7!, {r4}
clean_caches:
	/* Clean Data or unified cache to POU*/
	/* How to invalidate only L1 cache???? - #FIX_ME# */
	/* mcr	p15, 0, r11, c7, c11, 1 */
	cmp	r9, #1 /* Check whether L2 inval is required or not*/
	bne	skip_l2_inval
clean_l2:
	ldr r1, kernel_flush     /* get 32 bit addr of flush */
	mov lr, pc               /* prepare for return */
	bx  r1                   /* do it */
skip_l2_inval:

	/* Data memory barrier and Data sync barrier */
	mov     r1, #0
	mcr     p15, 0, r1, c7, c10, 4
	mcr     p15, 0, r1, c7, c10, 5

	wfi                             @ wait for interrupt

	nop
	nop
	nop
	bl wait_sdrc_ok
	/* restore regs and return */
	ldmfd   sp!, {r0-r12, pc}

/* Make sure SDRC accesses are ok */
wait_sdrc_ok:
	ldr 	r4, cm_idlest1_core
	ldr	r5, [r4]
	and 	r5, r5, #0x2
	cmp	r5, #0
	bne	wait_sdrc_ok
	ldr	r4, sdrc_power
	ldr	r5, [r4]
	bic 	r5, r5, #0x40
	str 	r5, [r4]
wait_dll_lock:
        /* Is dll in lock mode? */
	ldr     r4, sdrc_dlla_ctrl
	ldr     r5, [r4]
	tst	r5, #0x4
	bxne	lr
        /* wait till dll locks */
	ldr	r4, sdrc_dlla_status
	ldr	r5, [r4]
	and 	r5, r5, #0x4
	cmp	r5, #0x4
	bne	wait_dll_lock
	bx	lr
#ifdef CONFIG_ENABLE_OFF_MODE_JTAG_ETM_DEBUG
	/*
	 * unlock debug:
	 *  Input:
	 *      r6 has base address of emulation
	 *      r4 has unlock key
	 *  Output
	 *      r5 has PDS value (1=accessable)
	 */
unlock_debug:
	ldr     r5, [r6, #0xfb4]     /* get LSR */
	cmp     r5, #0x3             /* need unlocking? */
	streq   r4, [r6, #0xfb0]     /* unlock if so */
	ldr     r5, [r6, #0x314]     /* clear power status */
	bx      lr                   /* back to caller */

debug_vbase:
        .word CORTEX_EMU_DEBUG_V
debug_pbase:
        .word CORTEX_EMU_DEBUG_P
etm_vbase:
        .word CORTEX_EMU_ETM_V
etm_pbase:
        .word CORTEX_EMU_ETM_P
debug_xlar_key:
        .word 0xC5ACCE55
#endif

kernel_flush:
        .word v7_flush_dcache_all
cm_idlest1_core:
	.word CM_IDLEST1_CORE_V
cm_iclken1_core:
	.word CM_ICLKEN1_CORE_V
sdrc_dlla_status:
	.word SDRC_DLLA_STATUS_V
sdrc_dlla_ctrl:
	.word SDRC_DLLA_CTRL_V
pm_prepwstst_core:
	.word	PM_PREPWSTST_CORE_V
pm_prepwstst_core_p:
	.word 	PM_PREPWSTST_CORE_P
pm_prepwstst_mpu:
	.word	PM_PREPWSTST_MPU_V
pm_pwstctrl_mpu:
	.word	PM_PWSTCTRL_MPU_P
scratchpad_base:
	.word	SCRATCHPAD_BASE_P
sram_base:
	.word	SRAM_BASE_P
sdrc_power:
	.word SDRC_POWER_V
assoc_mask:
	.word	0x3ff
numset_mask:
	.word	0x7fff
ttbrbit_mask:
	.word	0xFFFFC000
table_index_mask:
	.word	0xFFF00000
table_entry:
	.word	0x00000C02
cache_pred_disable_mask:
	.word	0xFFFFE7FB
control_stat:
	.word	CONTROL_STAT_P
ENTRY(omap34xx_cpu_suspend_sz)
	.word	. - omap34xx_cpu_suspend
