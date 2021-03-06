/*
 * linux/arch/arm/mach-omap2/devices.c
 *
 * OMAP2 platform device setup/initialization
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>

#include <mach/control.h>
#include <mach/tc.h>
#include <mach/board.h>
#include <mach/mux.h>
#include <mach/gpio.h>
#include <mach/eac.h>
#include <mach/mmc.h>
#include <mach/board-archos.h>

#if defined(CONFIG_VIDEO_OMAP2) || defined(CONFIG_VIDEO_OMAP2_MODULE)

static struct resource cam_resources[] = {
	{
		.start		= OMAP24XX_CAMERA_BASE,
		.end		= OMAP24XX_CAMERA_BASE + 0xfff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap_cam_device = {
	.name		= "omap24xxcam",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(cam_resources),
	.resource	= cam_resources,
};

static inline void omap_init_camera(void)
{
	platform_device_register(&omap_cam_device);
}

#elif defined(CONFIG_VIDEO_OMAP3) || defined(CONFIG_VIDEO_OMAP3_MODULE)

static struct resource cam_resources[] = {
	{
		.start		= OMAP34XX_CAMERA_BASE,
		.end		= OMAP34XX_CAMERA_BASE + 0x1B70,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_34XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap_cam_device = {
	.name		= "omap34xxcam",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(cam_resources),
	.resource	= cam_resources,
};

static inline void omap_init_camera(void)
{
	platform_device_register(&omap_cam_device);
}
#else
static inline void omap_init_camera(void)
{
}
#endif

#if defined(CONFIG_OMAP_DSP) || defined(CONFIG_OMAP_DSP_MODULE)
#define OMAP2_MBOX_BASE		IO_ADDRESS(OMAP24XX_MAILBOX_BASE)

static struct resource mbox_resources[] = {
	{
		.start		= OMAP2_MBOX_BASE,
		.end		= OMAP2_MBOX_BASE + 0x11f,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_MAIL_U0_MPU,
		.flags		= IORESOURCE_IRQ,
	},
	{
		.start		= INT_24XX_MAIL_U3_MPU,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device mbox_device = {
	.name		= "mailbox",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mbox_resources),
	.resource	= mbox_resources,
};

static inline void omap_init_mbox(void)
{
	platform_device_register(&mbox_device);
}
#else
static inline void omap_init_mbox(void) { }
#endif

#if defined(CONFIG_OMAP_STI)

#if defined(CONFIG_ARCH_OMAP2)

#define OMAP2_STI_BASE		0x48068000
#define OMAP2_STI_CHANNEL_BASE	0x54000000
#define OMAP2_STI_IRQ		4

static struct resource sti_resources[] = {
	{
		.start		= OMAP2_STI_BASE,
		.end		= OMAP2_STI_BASE + 0x7ff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP2_STI_CHANNEL_BASE,
		.end		= OMAP2_STI_CHANNEL_BASE + SZ_64K - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP2_STI_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};
#elif defined(CONFIG_ARCH_OMAP3)

#define OMAP3_SDTI_BASE		0x54500000
#define OMAP3_SDTI_CHANNEL_BASE	0x54600000

static struct resource sti_resources[] = {
	{
		.start		= OMAP3_SDTI_BASE,
		.end		= OMAP3_SDTI_BASE + 0xFFF,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3_SDTI_CHANNEL_BASE,
		.end		= OMAP3_SDTI_CHANNEL_BASE + SZ_1M - 1,
		.flags		= IORESOURCE_MEM,
	}
};

#endif

static struct platform_device sti_device = {
	.name		= "sti",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(sti_resources),
	.resource	= sti_resources,
};

static inline void omap_init_sti(void)
{
	platform_device_register(&sti_device);
}
#else
static inline void omap_init_sti(void) {}
#endif

#if defined(CONFIG_SPI_OMAP24XX) || defined(CONFIG_SPI_OMAP24XX_MODULE)

#include <mach/mcspi.h>

#define OMAP2_MCSPI1_BASE		0x48098000
#define OMAP2_MCSPI2_BASE		0x4809a000
#define OMAP2_MCSPI3_BASE		0x480b8000
#define OMAP2_MCSPI4_BASE		0x480ba000

#define OMAP2_MCSPI_MASTER		0
#define OMAP2_MCSPI_SLAVE		1

static struct omap2_mcspi_platform_config omap2_mcspi1_config = {
	.num_cs		= 4,
	.force_cs_mode	= 1,
};

static struct resource omap2_mcspi1_resources[] = {
	{
		.start		= OMAP2_MCSPI1_BASE,
		.end		= OMAP2_MCSPI1_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},

	{
		.start		= INT_24XX_SPI1_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

struct platform_device omap2_mcspi1 = {
	.name		= "omap2_mcspi",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(omap2_mcspi1_resources),
	.resource	= omap2_mcspi1_resources,
	.dev		= {
		.platform_data = &omap2_mcspi1_config,
	},
};

static struct omap2_mcspi_platform_config omap2_mcspi2_config = {
	.num_cs		= 2,
	.mode		= OMAP2_MCSPI_MASTER,
	.dma_mode	= 1,
	.force_cs_mode	= 0,
	.fifo_depth	= 0,
};

static struct resource omap2_mcspi2_resources[] = {
	{
		.start		= OMAP2_MCSPI2_BASE,
		.end		= OMAP2_MCSPI2_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},

	{
		.start		= INT_24XX_SPI2_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

struct platform_device omap2_mcspi2 = {
	.name		= "omap2_mcspi",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(omap2_mcspi2_resources),
	.resource	= omap2_mcspi2_resources,
	.dev		= {
		.platform_data = &omap2_mcspi2_config,
	},
};

#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3)
static struct omap2_mcspi_platform_config omap2_mcspi3_config = {
	.num_cs		= 2,
};

static struct resource omap2_mcspi3_resources[] = {
	{
		.start		= OMAP2_MCSPI3_BASE,
		.end		= OMAP2_MCSPI3_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},

	{
		.start		= INT_24XX_SPI3_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device omap2_mcspi3 = {
	.name		= "omap2_mcspi",
	.id		= 3,
	.num_resources	= ARRAY_SIZE(omap2_mcspi3_resources),
	.resource	= omap2_mcspi3_resources,
	.dev		= {
		.platform_data = &omap2_mcspi3_config,
	},
};
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct omap2_mcspi_platform_config omap2_mcspi4_config = {
	.num_cs		= 1,
};

static struct resource omap2_mcspi4_resources[] = {
	{
		.start		= OMAP2_MCSPI4_BASE,
		.end		= OMAP2_MCSPI4_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},

	{
		.start		= INT_34XX_SPI4_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device omap2_mcspi4 = {
	.name		= "omap2_mcspi",
	.id		= 4,
	.num_resources	= ARRAY_SIZE(omap2_mcspi4_resources),
	.resource	= omap2_mcspi4_resources,
	.dev		= {
		.platform_data = &omap2_mcspi4_config,
	},
};
#endif

static void omap_init_mcspi(void)
{
	platform_device_register(&omap2_mcspi1);
	platform_device_register(&omap2_mcspi2);
#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3)
	if (cpu_is_omap2430() || cpu_is_omap343x())
		platform_device_register(&omap2_mcspi3);
#endif
#ifdef CONFIG_ARCH_OMAP3
	if (cpu_is_omap343x())
		platform_device_register(&omap2_mcspi4);
#endif
}

#else
static inline void omap_init_mcspi(void) {}
#endif

#ifdef CONFIG_SND_OMAP24XX_EAC

#define OMAP2_EAC_BASE			(L4_24XX_BASE + 0x90000)

static struct resource omap2_eac_resources[] = {
	{
		.start		= OMAP2_EAC_BASE,
		.end		= OMAP2_EAC_BASE + 0xfff,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device omap2_eac_device = {
	.name		= "omap24xx-eac",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap2_eac_resources),
	.resource	= omap2_eac_resources,
	.dev = {
		.platform_data = NULL,
	},
};

void omap_init_eac(struct eac_platform_data *pdata)
{
	omap2_eac_device.dev.platform_data = pdata;
	platform_device_register(&omap2_eac_device);
}

#else
void omap_init_eac(struct eac_platform_data *pdata) {}
#endif

#ifdef CONFIG_OMAP_SHA1_MD5
static struct resource sha1_md5_resources[] = {
	{
		.start	= OMAP24XX_SEC_SHA1MD5_BASE,
		.end	= OMAP24XX_SEC_SHA1MD5_BASE + 0x64,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_24XX_SHA1MD5,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device sha1_md5_device = {
	.name		= "OMAP SHA1/MD5",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(sha1_md5_resources),
	.resource	= sha1_md5_resources,
};

static void omap_init_sha1_md5(void)
{
	platform_device_register(&sha1_md5_device);
}
#else
static inline void omap_init_sha1_md5(void) { }
#endif

/*-------------------------------------------------------------------------*/

#if	defined(CONFIG_MMC_OMAP) || defined(CONFIG_MMC_OMAP_MODULE) || \
	defined(CONFIG_MMC_OMAP_HS) || defined(CONFIG_MMC_OMAP_HS_MODULE)

static inline void omap2_mmc_mux(struct omap_mmc_platform_data *mmc_controller,
			int controller_nr)
{
	if (cpu_is_omap2420() && controller_nr == 0) {
		omap_cfg_reg(H18_24XX_MMC_CMD);
		omap_cfg_reg(H15_24XX_MMC_CLKI);
		omap_cfg_reg(G19_24XX_MMC_CLKO);
		omap_cfg_reg(F20_24XX_MMC_DAT0);
		omap_cfg_reg(F19_24XX_MMC_DAT_DIR0);
		omap_cfg_reg(G18_24XX_MMC_CMD_DIR);
		if (mmc_controller->slots[0].wire4) {
			omap_cfg_reg(H14_24XX_MMC_DAT1);
			omap_cfg_reg(E19_24XX_MMC_DAT2);
			omap_cfg_reg(D19_24XX_MMC_DAT3);
			omap_cfg_reg(E20_24XX_MMC_DAT_DIR1);
			omap_cfg_reg(F18_24XX_MMC_DAT_DIR2);
			omap_cfg_reg(E18_24XX_MMC_DAT_DIR3);
		}

		/*
		 * Use internal loop-back in MMC/SDIO Module Input Clock
		 * selection
		 */
		if (mmc_controller->slots[0].internal_clock) {
			u32 v = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
			v |= (1 << 24);
			omap_ctrl_writel(v, OMAP2_CONTROL_DEVCONF0);
		}
	}
	if (cpu_is_omap3430()) {
		if (controller_nr == 0) {
			omap_cfg_reg(N28_3430_MMC1_CLK);
			omap_cfg_reg(M27_3430_MMC1_CMD);
			omap_cfg_reg(N27_3430_MMC1_DAT0);
			omap_cfg_reg(N26_3430_MMC1_DAT1);
			omap_cfg_reg(N25_3430_MMC1_DAT2);
			omap_cfg_reg(P28_3430_MMC1_DAT3);
		if (!machine_has_micro_sd() )
			{
			omap_cfg_reg(P27_3430_MMC1_DAT4);
			omap_cfg_reg(P26_3430_MMC1_DAT5);
			omap_cfg_reg(R27_3430_MMC1_DAT6);
			omap_cfg_reg(R25_3430_MMC1_DAT7);
			}
		}
		if (controller_nr == 1) {
			/* MMC2 */
			omap_cfg_reg(AE2_3430_MMC2_CLK);
			omap_cfg_reg(AG5_3430_MMC2_CMD);
			omap_cfg_reg(AH5_3430_MMC2_DAT0);
			omap_cfg_reg(AH4_3430_MMC2_DAT1);
			omap_cfg_reg(AG4_3430_MMC2_DAT2);
			omap_cfg_reg(AF4_3430_MMC2_DAT3);
		}
		if (controller_nr == 2) {
			/* MMC3 */
			omap_cfg_reg(AF10_3430_MMC3_CLK);
			omap_cfg_reg(AC3_3430_MMC3_CMD);
			omap_cfg_reg(AE11_3430_MMC3_DAT0);
			omap_cfg_reg(AH9_3430_MMC3_DAT1);
			omap_cfg_reg(AF13_3430_MMC3_DAT2);
			omap_cfg_reg(AE13_3430_MMC3_DAT3);
		}
	}
}

void __init omap2_init_mmc(struct omap_mmc_platform_data **mmc_data,
			int nr_controllers)
{
	int i;

	for (i = 0; i < nr_controllers; i++) {
		unsigned long base, size;
		unsigned int irq = 0;

		if (!mmc_data[i])
			continue;

		omap2_mmc_mux(mmc_data[i], i);

		switch (i) {
		case 0:
			base = OMAP2_MMC1_BASE;
			irq = INT_24XX_MMC_IRQ;
			break;
		case 1:
			base = OMAP2_MMC2_BASE;
			irq = INT_24XX_MMC2_IRQ;
			break;
		case 2:
			if (!cpu_is_omap34xx())
				return;
			base = OMAP3_MMC3_BASE;
			irq = INT_34XX_MMC3_IRQ;
			break;
		default:
			continue;
		}

		if (cpu_is_omap2420())
			size = OMAP2420_MMC_SIZE;
		else
			size = HSMMC_SIZE;

		omap_mmc_add(i, base, size, irq, mmc_data[i]);
	};
}

#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_HDQ_MASTER_OMAP) || defined(CONFIG_HDQ_MASTER_OMAP_MODULE)
#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3430)
#define OMAP_HDQ_BASE	0x480B2000
#endif
static struct resource omap_hdq_resources[] = {
	{
		.start		= OMAP_HDQ_BASE,
		.end		= OMAP_HDQ_BASE + 0x1C,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_HDQ_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};
static struct platform_device omap_hdq_dev = {
	.name = "omap_hdq",
	.id = 0,
	.dev = {
		.platform_data = NULL,
	},
	.num_resources	= ARRAY_SIZE(omap_hdq_resources),
	.resource	= omap_hdq_resources,
};
static inline void omap_hdq_init(void)
{
	(void) platform_device_register(&omap_hdq_dev);
}
#else
static inline void omap_hdq_init(void) {}
#endif

/*-------------------------------------------------------------------------*/

static int __init omap2_init_devices(void)
{
	/* please keep these calls, and their implementations above,
	 * in alphabetical order so they're easier to sort through.
	 */
	omap_init_camera();
	omap_init_mbox();
	omap_init_mcspi();
	omap_hdq_init();
	omap_init_sti();
	omap_init_sha1_md5();

	return 0;
}
arch_initcall(omap2_init_devices);
