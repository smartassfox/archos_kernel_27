/*
 * SH7206 Setup
 *
 *  Copyright (C) 2006  Yoshinori Sato
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/serial_sci.h>

enum {
	UNUSED = 0,

	/* interrupt sources */
	IRQ0, IRQ1, IRQ2, IRQ3, IRQ4, IRQ5, IRQ6, IRQ7,
	PINT0, PINT1, PINT2, PINT3, PINT4, PINT5, PINT6, PINT7,
	ADC_ADI0, ADC_ADI1,
	DMAC0_DEI, DMAC0_HEI, DMAC1_DEI, DMAC1_HEI,
	DMAC2_DEI, DMAC2_HEI, DMAC3_DEI, DMAC3_HEI,
	DMAC4_DEI, DMAC4_HEI, DMAC5_DEI, DMAC5_HEI,
	DMAC6_DEI, DMAC6_HEI, DMAC7_DEI, DMAC7_HEI,
	CMT0, CMT1, BSC, WDT,
	MTU2_TGI0A, MTU2_TGI0B, MTU2_TGI0C, MTU2_TGI0D,
	MTU2_TCI0V, MTU2_TGI0E, MTU2_TGI0F,
	MTU2_TGI1A, MTU2_TGI1B, MTU2_TCI1V, MTU2_TCI1U,
	MTU2_TGI2A, MTU2_TGI2B, MTU2_TCI2V, MTU2_TCI2U,
	MTU2_TGI3A, MTU2_TGI3B, MTU2_TGI3C, MTU2_TGI3D, MTU2_TCI3V,
	MTU2_TGI4A, MTU2_TGI4B, MTU2_TGI4C, MTU2_TGI4D, MTU2_TCI4V,
	MTU2_TGI5U, MTU2_TGI5V, MTU2_TGI5W,
	POE2_OEI1, POE2_OEI2,
	MTU2S_TGI3A, MTU2S_TGI3B, MTU2S_TGI3C, MTU2S_TGI3D, MTU2S_TCI3V,
	MTU2S_TGI4A, MTU2S_TGI4B, MTU2S_TGI4C, MTU2S_TGI4D, MTU2S_TCI4V,
	MTU2S_TGI5U, MTU2S_TGI5V, MTU2S_TGI5W,
	POE2_OEI3,
	IIC3_STPI, IIC3_NAKI, IIC3_RXI, IIC3_TXI, IIC3_TEI,
	SCIF0_BRI, SCIF0_ERI, SCIF0_RXI, SCIF0_TXI,
	SCIF1_BRI, SCIF1_ERI, SCIF1_RXI, SCIF1_TXI,
	SCIF2_BRI, SCIF2_ERI, SCIF2_RXI, SCIF2_TXI,
	SCIF3_BRI, SCIF3_ERI, SCIF3_RXI, SCIF3_TXI,

	/* interrupt groups */
	PINT, DMAC0, DMAC1, DMAC2, DMAC3, DMAC4, DMAC5, DMAC6, DMAC7,
	MTU0_ABCD, MTU0_VEF, MTU1_AB, MTU1_VU, MTU2_AB, MTU2_VU,
	MTU3_ABCD, MTU4_ABCD, MTU5, POE2_12, MTU3S_ABCD, MTU4S_ABCD, MTU5S,
	IIC3, SCIF0, SCIF1, SCIF2, SCIF3,
};

static struct intc_vect vectors[] __initdata = {
	INTC_IRQ(IRQ0, 64), INTC_IRQ(IRQ1, 65),
	INTC_IRQ(IRQ2, 66), INTC_IRQ(IRQ3, 67),
	INTC_IRQ(IRQ4, 68), INTC_IRQ(IRQ5, 69),
	INTC_IRQ(IRQ6, 70), INTC_IRQ(IRQ7, 71),
	INTC_IRQ(PINT0, 80), INTC_IRQ(PINT1, 81),
	INTC_IRQ(PINT2, 82), INTC_IRQ(PINT3, 83),
	INTC_IRQ(PINT4, 84), INTC_IRQ(PINT5, 85),
	INTC_IRQ(PINT6, 86), INTC_IRQ(PINT7, 87),
	INTC_IRQ(ADC_ADI0, 92), INTC_IRQ(ADC_ADI1, 96),
	INTC_IRQ(DMAC0_DEI, 108), INTC_IRQ(DMAC0_HEI, 109),
	INTC_IRQ(DMAC1_DEI, 112), INTC_IRQ(DMAC1_HEI, 113),
	INTC_IRQ(DMAC2_DEI, 116), INTC_IRQ(DMAC2_HEI, 117),
	INTC_IRQ(DMAC3_DEI, 120), INTC_IRQ(DMAC3_HEI, 121),
	INTC_IRQ(DMAC4_DEI, 124), INTC_IRQ(DMAC4_HEI, 125),
	INTC_IRQ(DMAC5_DEI, 128), INTC_IRQ(DMAC5_HEI, 129),
	INTC_IRQ(DMAC6_DEI, 132), INTC_IRQ(DMAC6_HEI, 133),
	INTC_IRQ(DMAC7_DEI, 136), INTC_IRQ(DMAC7_HEI, 137),
	INTC_IRQ(CMT0, 140), INTC_IRQ(CMT1, 144),
	INTC_IRQ(BSC, 148), INTC_IRQ(WDT, 152),
	INTC_IRQ(MTU2_TGI0A, 156), INTC_IRQ(MTU2_TGI0B, 157),
	INTC_IRQ(MTU2_TGI0C, 158), INTC_IRQ(MTU2_TGI0D, 159),
	INTC_IRQ(MTU2_TCI0V, 160),
	INTC_IRQ(MTU2_TGI0E, 161), INTC_IRQ(MTU2_TGI0F, 162),
	INTC_IRQ(MTU2_TGI1A, 164), INTC_IRQ(MTU2_TGI1B, 165),
	INTC_IRQ(MTU2_TCI1V, 168), INTC_IRQ(MTU2_TCI1U, 169),
	INTC_IRQ(MTU2_TGI2A, 172), INTC_IRQ(MTU2_TGI2B, 173),
	INTC_IRQ(MTU2_TCI2V, 176), INTC_IRQ(MTU2_TCI2U, 177),
	INTC_IRQ(MTU2_TGI3A, 180), INTC_IRQ(MTU2_TGI3B, 181),
	INTC_IRQ(MTU2_TGI3C, 182), INTC_IRQ(MTU2_TGI3D, 183),
	INTC_IRQ(MTU2_TCI3V, 184),
	INTC_IRQ(MTU2_TGI4A, 188), INTC_IRQ(MTU2_TGI4B, 189),
	INTC_IRQ(MTU2_TGI4C, 190), INTC_IRQ(MTU2_TGI4D, 191),
	INTC_IRQ(MTU2_TCI4V, 192),
	INTC_IRQ(MTU2_TGI5U, 196), INTC_IRQ(MTU2_TGI5V, 197),
	INTC_IRQ(MTU2_TGI5W, 198),
	INTC_IRQ(POE2_OEI1, 200), INTC_IRQ(POE2_OEI2, 201),
	INTC_IRQ(MTU2S_TGI3A, 204), INTC_IRQ(MTU2S_TGI3B, 205),
	INTC_IRQ(MTU2S_TGI3C, 206), INTC_IRQ(MTU2S_TGI3D, 207),
	INTC_IRQ(MTU2S_TCI3V, 208),
	INTC_IRQ(MTU2S_TGI4A, 212), INTC_IRQ(MTU2S_TGI4B, 213),
	INTC_IRQ(MTU2S_TGI4C, 214), INTC_IRQ(MTU2S_TGI4D, 215),
	INTC_IRQ(MTU2S_TCI4V, 216),
	INTC_IRQ(MTU2S_TGI5U, 220), INTC_IRQ(MTU2S_TGI5V, 221),
	INTC_IRQ(MTU2S_TGI5W, 222),
	INTC_IRQ(POE2_OEI3, 224),
	INTC_IRQ(IIC3_STPI, 228), INTC_IRQ(IIC3_NAKI, 229),
	INTC_IRQ(IIC3_RXI, 230), INTC_IRQ(IIC3_TXI, 231),
	INTC_IRQ(IIC3_TEI, 232),
	INTC_IRQ(SCIF0_BRI, 240), INTC_IRQ(SCIF0_ERI, 241),
	INTC_IRQ(SCIF0_RXI, 242), INTC_IRQ(SCIF0_TXI, 243),
	INTC_IRQ(SCIF1_BRI, 244), INTC_IRQ(SCIF1_ERI, 245),
	INTC_IRQ(SCIF1_RXI, 246), INTC_IRQ(SCIF1_TXI, 247),
	INTC_IRQ(SCIF2_BRI, 248), INTC_IRQ(SCIF2_ERI, 249),
	INTC_IRQ(SCIF2_RXI, 250), INTC_IRQ(SCIF2_TXI, 251),
	INTC_IRQ(SCIF3_BRI, 252), INTC_IRQ(SCIF3_ERI, 253),
	INTC_IRQ(SCIF3_RXI, 254), INTC_IRQ(SCIF3_TXI, 255),
};

static struct intc_group groups[] __initdata = {
	INTC_GROUP(PINT, PINT0, PINT1, PINT2, PINT3,
		   PINT4, PINT5, PINT6, PINT7),
	INTC_GROUP(DMAC0, DMAC0_DEI, DMAC0_HEI),
	INTC_GROUP(DMAC1, DMAC1_DEI, DMAC1_HEI),
	INTC_GROUP(DMAC2, DMAC2_DEI, DMAC2_HEI),
	INTC_GROUP(DMAC3, DMAC3_DEI, DMAC3_HEI),
	INTC_GROUP(DMAC4, DMAC4_DEI, DMAC4_HEI),
	INTC_GROUP(DMAC5, DMAC5_DEI, DMAC5_HEI),
	INTC_GROUP(DMAC6, DMAC6_DEI, DMAC6_HEI),
	INTC_GROUP(DMAC7, DMAC7_DEI, DMAC7_HEI),
	INTC_GROUP(MTU0_ABCD, MTU2_TGI0A, MTU2_TGI0B, MTU2_TGI0C, MTU2_TGI0D),
	INTC_GROUP(MTU0_VEF, MTU2_TCI0V, MTU2_TGI0E, MTU2_TGI0F),
	INTC_GROUP(MTU1_AB, MTU2_TGI1A, MTU2_TGI1B),
	INTC_GROUP(MTU1_VU, MTU2_TCI1V, MTU2_TCI1U),
	INTC_GROUP(MTU2_AB, MTU2_TGI2A, MTU2_TGI2B),
	INTC_GROUP(MTU2_VU, MTU2_TCI2V, MTU2_TCI2U),
	INTC_GROUP(MTU3_ABCD, MTU2_TGI3A, MTU2_TGI3B, MTU2_TGI3C, MTU2_TGI3D),
	INTC_GROUP(MTU4_ABCD, MTU2_TGI4A, MTU2_TGI4B, MTU2_TGI4C, MTU2_TGI4D),
	INTC_GROUP(MTU5, MTU2_TGI5U, MTU2_TGI5V, MTU2_TGI5W),
	INTC_GROUP(POE2_12, POE2_OEI1, POE2_OEI2),
	INTC_GROUP(MTU3S_ABCD, MTU2S_TGI3A, MTU2S_TGI3B,
		   MTU2S_TGI3C, MTU2S_TGI3D),
	INTC_GROUP(MTU4S_ABCD, MTU2S_TGI4A, MTU2S_TGI4B,
		   MTU2S_TGI4C, MTU2S_TGI4D),
	INTC_GROUP(MTU5S, MTU2S_TGI5U, MTU2S_TGI5V, MTU2S_TGI5W),
	INTC_GROUP(IIC3, IIC3_STPI, IIC3_NAKI, IIC3_RXI, IIC3_TXI, IIC3_TEI),
	INTC_GROUP(SCIF0, SCIF0_BRI, SCIF0_ERI, SCIF0_RXI, SCIF0_TXI),
	INTC_GROUP(SCIF1, SCIF1_BRI, SCIF1_ERI, SCIF1_RXI, SCIF1_TXI),
	INTC_GROUP(SCIF2, SCIF2_BRI, SCIF2_ERI, SCIF2_RXI, SCIF2_TXI),
	INTC_GROUP(SCIF3, SCIF3_BRI, SCIF3_ERI, SCIF3_RXI, SCIF3_TXI),
};

static struct intc_prio_reg prio_registers[] __initdata = {
	{ 0xfffe0818, 0, 16, 4, /* IPR01 */ { IRQ0, IRQ1, IRQ2, IRQ3 } },
	{ 0xfffe081a, 0, 16, 4, /* IPR02 */ { IRQ4, IRQ5, IRQ6, IRQ7 } },
	{ 0xfffe0820, 0, 16, 4, /* IPR05 */ { PINT, 0, ADC_ADI0, ADC_ADI1 } },
	{ 0xfffe0c00, 0, 16, 4, /* IPR06 */ { DMAC0, DMAC1, DMAC2, DMAC3 } },
	{ 0xfffe0c02, 0, 16, 4, /* IPR07 */ { DMAC4, DMAC5, DMAC6, DMAC7 } },
	{ 0xfffe0c04, 0, 16, 4, /* IPR08 */ { CMT0, CMT1, BSC, WDT } },
	{ 0xfffe0c06, 0, 16, 4, /* IPR09 */ { MTU0_ABCD, MTU0_VEF,
					      MTU1_AB, MTU1_VU } },
	{ 0xfffe0c08, 0, 16, 4, /* IPR10 */ { MTU2_AB, MTU2_VU,
					      MTU3_ABCD, MTU2_TCI3V } },
	{ 0xfffe0c0a, 0, 16, 4, /* IPR11 */ { MTU4_ABCD, MTU2_TCI4V,
					      MTU5, POE2_12 } },
	{ 0xfffe0c0c, 0, 16, 4, /* IPR12 */ { MTU3S_ABCD, MTU2S_TCI3V,
					      MTU4S_ABCD, MTU2S_TCI4V } },
	{ 0xfffe0c0e, 0, 16, 4, /* IPR13 */ { MTU5S, POE2_OEI3, IIC3, 0 } },
	{ 0xfffe0c10, 0, 16, 4, /* IPR14 */ { SCIF0, SCIF1, SCIF2, SCIF3 } },
};

static struct intc_mask_reg mask_registers[] __initdata = {
	{ 0xfffe0808, 0, 16, /* PINTER */
	  { 0, 0, 0, 0, 0, 0, 0, 0,
	    PINT7, PINT6, PINT5, PINT4, PINT3, PINT2, PINT1, PINT0 } },
};

static DECLARE_INTC_DESC(intc_desc, "sh7206", vectors, groups,
			 mask_registers, prio_registers, NULL);

static struct plat_sci_port sci_platform_data[] = {
	{
		.mapbase	= 0xfffe8000,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCIF,
		.irqs		=  { 241, 242, 243, 240 },
	}, {
		.mapbase	= 0xfffe8800,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCIF,
		.irqs		=  { 245, 246, 247, 244 },
	}, {
		.mapbase	= 0xfffe9000,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCIF,
		.irqs		=  { 249, 250, 251, 248 },
	}, {
		.mapbase	= 0xfffe9800,
		.flags		= UPF_BOOT_AUTOCONF,
		.type		= PORT_SCIF,
		.irqs		=  { 253, 254, 255, 252 },
	}, {
		.flags = 0,
	}
};

static struct platform_device sci_device = {
	.name		= "sh-sci",
	.id		= -1,
	.dev		= {
		.platform_data	= sci_platform_data,
	},
};

static struct platform_device *sh7206_devices[] __initdata = {
	&sci_device,
};

static int __init sh7206_devices_setup(void)
{
	return platform_add_devices(sh7206_devices,
				    ARRAY_SIZE(sh7206_devices));
}
__initcall(sh7206_devices_setup);

void __init plat_irq_setup(void)
{
	register_intc_controller(&intc_desc);
}
