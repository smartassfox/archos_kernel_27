#ifndef ____ASM_ARCH_SDRC_H
#define ____ASM_ARCH_SDRC_H

/*
 * OMAP2/3 SDRC/SMS register definitions
 *
 * Copyright (C) 2007-2008 Texas Instruments, Inc.
 * Copyright (C) 2007-2008 Nokia Corporation
 *
 * Tony Lindgren
 * Paul Walmsley
 * Richard Woodruff
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <mach/io.h>

/* SDRC register offsets - read/write with sdrc_{read,write}_reg() */

#define SDRC_SYSCONFIG		0x010
#define SDRC_CS_CFG		0x040
#define SDRC_SHARING		0x044
#define SDRC_ERR_TYPE		0x04C
#define SDRC_DLLA_CTRL		0x060
#define SDRC_DLLA_STATUS	0x064
#define SDRC_DLLB_CTRL		0x068
#define SDRC_DLLB_STATUS	0x06C
#define SDRC_POWER		0x070
#define SDRC_MCFG_0		0x080
#define SDRC_MR_0		0x084
#define SDRC_ACTIM_CTRL_A_0	0x09c
#define SDRC_ACTIM_CTRL_B_0	0x0a0
#define SDRC_RFR_CTRL_0		0x0a4
#define SDRC_MCFG_1		0x0B0
#define SDRC_MR_1		0x0B4
#define SDRC_ACTIM_CTRL_A_1	0x0C4
#define SDRC_ACTIM_CTRL_B_1	0x0C8
#define SDRC_RFR_CTRL_1		0x0D4

/* SDRC_POWER Register value and mask */
#define SDRC_POWER_PWDENA_OFFSET	(2)
#define SDRC_POWER_CLKCTRL_OFFSET	(4)
#define SDRC_POWER_SRFRONIDLEREQ_OFFSET	(6)
#define SDRC_POWER_AUTOCOUNT_OFFSET	(8)
#define SDRC_POWER_WAKEUPPROC_OFFSET 	(26)

#define SDRC_POWER_PWDENA_MASK		(0x1 << SDRC_POWER_PWDENA_OFFSET)
#define SDRC_POWER_PWDENA_ON		(1 << SDRC_POWER_PWDENA_OFFSET);

#define SDRC_POWER_CLKCTRL_MASK		(0x3 << SDRC_POWER_CLKCTRL_OFFSET)
#define SDRC_POWER_CLKCTRL_INTERNAL	(0x1 << SDRC_POWER_CLKCTRL_OFFSET)
#define SDRC_POWER_CLKCTRL_SELF_REFRESH	(0x2 << SDRC_POWER_CLKCTRL_OFFSET)

#define	SDRC_POWER_SRFRONIDLEREQ_MASK	(0x1 << SDRC_POWER_SRFRONIDLEREQ_OFFSET)
#define SDRC_POWER_SRFRONIDLEREQ_ON	(0x1 << SDRC_POWER_SRFRONIDLEREQ_OFFSET)

#define SDRC_POWER_AUTOCOUNT_MASK		(0xFFFF << SDRC_POWER_AUTOCOUNT_OFFSET)

#define SDRC_POWER_WAKEUPPROC_MASK	(0x1 << SDRC_POWER_WAKEUPPROC_OFFSET)

/* SDRC_DLLA_CTRL Register value and mask */
#define SDRC_DLLA_DLLMODEONIDLEREQ_OFFSET	(5)
#define SDRC_DLLA_CTRL_FIXEDDELAY_OFFSET	(24)

#define SDRC_DLLA_DLLMODEONIDLEREQ_MASK 	(0x3 << SDRC_DLLA_DLLMODEONIDLEREQ_OFFSET)
#define SDRC_DLLA_DLLMODEONIDLEREQ_DOWN		(0x0 << SDRC_DLLA_DLLMODEONIDLEREQ_OFFSET)
#define SDRC_DLLA_DLLMODEONIDLEREQ_IDLE		(0x1 << SDRC_DLLA_DLLMODEONIDLEREQ_OFFSET)
#define SDRC_DLLA_DLLMODEONIDLEREQ_NONE		(0x2 << SDRC_DLLA_DLLMODEONIDLEREQ_OFFSET)

#define SDRC_DLLA_CTRL_FIXEDDELAY_MASK		(0xFF << SDRC_DLLA_CTRL_FIXEDDELAY_OFFSET)


/*
 * These values represent the number of memory clock cycles between
 * autorefresh initiation.  They assume 1 refresh per 64 ms (JEDEC), 8192
 * rows per device, and include a subtraction of a 50 cycle window in the
 * event that the autorefresh command is delayed due to other SDRC activity.
 * The '| 1' sets the ARE field to send one autorefresh when the autorefresh
 * counter reaches 0.
 *
 * These represent optimal values for common parts, it won't work for all.
 * As long as you scale down, most parameters are still work, they just
 * become sub-optimal. The RFR value goes in the opposite direction. If you
 * don't adjust it down as your clock period increases the refresh interval
 * will not be met. Setting all parameters for complete worst case may work,
 * but may cut memory performance by 2x. Due to errata the DLLs need to be
 * unlocked and their value needs run time calibration.	A dynamic call is
 * need for that as no single right value exists acorss production samples.
 *
 * Only the FULL speed values are given. Current code is such that rate
 * changes must be made at DPLLoutx2. The actual value adjustment for low
 * frequency operation will be handled by omap_set_performance()
 *
 * By having the boot loader boot up in the fastest L4 speed available likely
 * will result in something which you can switch between.
 */
#define SDRC_RFR_CTRL_165MHz	(0x00044c00 | 1)
#define SDRC_RFR_CTRL_133MHz	(0x0003de00 | 1)
#define SDRC_RFR_CTRL_100MHz	(0x0002da01 | 1)
#define SDRC_RFR_CTRL_110MHz	(0x0002da01 | 1) /* Need to calc */
#define SDRC_RFR_CTRL_BYPASS	(0x00005000 | 1) /* Need to calc */


/*
 * SMS register access
 */

#define OMAP242X_SMS_REGADDR(reg)	IO_ADDRESS(OMAP2420_SMS_BASE + reg)
#define OMAP243X_SMS_REGADDR(reg)	IO_ADDRESS(OMAP243X_SMS_BASE + reg)
#define OMAP343X_SMS_REGADDR(reg)	IO_ADDRESS(OMAP343X_SMS_BASE + reg)

/* SMS register offsets - read/write with sms_{read,write}_reg() */

#define SMS_SYSCONFIG		0x010
#define SMS_CLASS_ARBITER0	0x0150
#define SMS_CLASS_ARBITER1	0x0154
#define SMS_CLASS_ARBITER2	0x0158
#define SMS_INTERCLASS_ARBITER	0x0160

/* REVISIT: fill in other SMS registers here */


#ifndef __ASSEMBLER__

/**
 * struct omap_sdrc_params - SDRC parameters for a given SDRC clock rate
 * @rate: SDRC clock rate (in Hz)
 * @actim_ctrla: Value to program to SDRC_ACTIM_CTRLA for this rate
 * @actim_ctrlb: Value to program to SDRC_ACTIM_CTRLB for this rate
 * @rfr_ctrl: Value to program to SDRC_RFR_CTRL for this rate
 * @mr: Value to program to SDRC_MR for this rate
 *
 * This structure holds a pre-computed set of register values for the
 * SDRC for a given SDRC clock rate and SDRAM chip.  These are
 * intended to be pre-computed and specified in an array in the board-*.c
 * files.  The structure is keyed off the 'rate' field.
 */
struct omap_sdrc_params {
	unsigned long rate;
	u32 actim_ctrla;
	u32 actim_ctrlb;
	u32 rfr_ctrl;
	u32 mr;
};

void __init omap2_sdrc_init(struct omap_sdrc_params *);
struct omap_sdrc_params *omap2_sdrc_get_params(unsigned long r);

#if defined (CONFIG_ARCH_OMAP2) || defined (CONFIG_ARCH_OMAP3)

struct memory_timings {
	u32 m_type;		/* ddr = 1, sdr = 0 */
	u32 dll_mode;		/* use lock mode = 1, unlock mode = 0 */
	u32 slow_dll_ctrl;	/* unlock mode, dll value for slow speed */
	u32 fast_dll_ctrl;	/* unlock mode, dll value for fast speed */
	u32 base_cs;		/* base chip select to use for calculations */
};

extern void omap2xxx_sdrc_init_params(u32 force_lock_to_unlock_mode);

u32 omap2xxx_sdrc_dll_is_unlocked(void);
u32 omap2xxx_sdrc_reprogram(u32 level, u32 force);

#endif  /* CONFIG_ARCH_OMAP2 */

#endif  /* __ASSEMBLER__ */

#endif
