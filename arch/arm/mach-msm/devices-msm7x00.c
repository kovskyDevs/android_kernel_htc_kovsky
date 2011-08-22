/* linux/arch/arm/mach-msm/devices.c
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>

#include <mach/irqs.h>
#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include <mach/gpio.h>
#include <mach/msm_hsusb.h>
#include "devices.h"

#include <asm/mach/flash.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include "clock.h"
#include <mach/mmc.h>

static struct resource resources_uart1[] = {
	{
		.start	= INT_UART1,
		.end	= INT_UART1,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_UART1_PHYS,
		.end	= MSM_UART1_PHYS + MSM_UART1_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource resources_uart2[] = {
	{
		.start	= INT_UART2,
		.end	= INT_UART2,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_UART2_PHYS,
		.end	= MSM_UART2_PHYS + MSM_UART2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource resources_uart3[] = {
	{
		.start	= INT_UART3,
		.end	= INT_UART3,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_UART3_PHYS,
		.end	= MSM_UART3_PHYS + MSM_UART3_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device msm_device_uart1 = {
	.name	= "msm_serial",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_uart1),
	.resource	= resources_uart1,
};

struct platform_device msm_device_uart2 = {
	.name	= "msm_serial",
	.id	= 1,
	.num_resources	= ARRAY_SIZE(resources_uart2),
	.resource	= resources_uart2,
};

struct platform_device msm_device_uart3 = {
	.name	= "msm_serial",
	.id	= 2,
	.num_resources	= ARRAY_SIZE(resources_uart3),
	.resource	= resources_uart3,
};

static struct resource msm_uart1_dm_resources[] = {
	{
		.start = MSM_UART1DM_PHYS,
		.end   = MSM_UART1DM_PHYS + PAGE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = INT_UART1DM_IRQ,
		.end   = INT_UART1DM_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = INT_UART1DM_RX,
		.end   = INT_UART1DM_RX,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = DMOV_HSUART1_TX_CHAN,
		.end   = DMOV_HSUART1_RX_CHAN,
		.name  = "uartdm_channels",
		.flags = IORESOURCE_DMA,
	},
	{
		.start = DMOV_HSUART1_TX_CRCI,
		.end   = DMOV_HSUART1_RX_CRCI,
		.name  = "uartdm_crci",
		.flags = IORESOURCE_DMA,
	},
};

static u64 msm_uart_dm1_dma_mask = DMA_BIT_MASK(32);

struct platform_device msm_device_uart_dm1 = {
	.name = "msm_serial_hs",
	.id = 0,
	.num_resources = ARRAY_SIZE(msm_uart1_dm_resources),
	.resource = msm_uart1_dm_resources,
	.dev		= {
		.dma_mask = &msm_uart_dm1_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static struct resource msm_uart2_dm_resources[] = {
	{
		.start = MSM_UART2DM_PHYS,
		.end   = MSM_UART2DM_PHYS + PAGE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = INT_UART2DM_IRQ,
		.end   = INT_UART2DM_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = INT_UART2DM_RX,
		.end   = INT_UART2DM_RX,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = DMOV_HSUART2_TX_CHAN,
		.end   = DMOV_HSUART2_RX_CHAN,
		.name  = "uartdm_channels",
		.flags = IORESOURCE_DMA,
	},
	{
		.start = DMOV_HSUART2_TX_CRCI,
		.end   = DMOV_HSUART2_RX_CRCI,
		.name  = "uartdm_crci",
		.flags = IORESOURCE_DMA,
	},
};

static u64 msm_uart_dm2_dma_mask = DMA_BIT_MASK(32);

struct platform_device msm_device_uart_dm2 = {
	.name = "msm_serial_hs",
	.id = 1,
	.num_resources = ARRAY_SIZE(msm_uart2_dm_resources),
	.resource = msm_uart2_dm_resources,
	.dev		= {
		.dma_mask = &msm_uart_dm2_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static struct resource resources_i2c[] = {
	{
		.start	= MSM_I2C_PHYS,
		.end	= MSM_I2C_PHYS + MSM_I2C_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_PWB_I2C,
		.end	= INT_PWB_I2C,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_i2c = {
	.name		= "msm_i2c",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_i2c),
	.resource	= resources_i2c,
};

#define GPIO_I2C_CLK 60
#define GPIO_I2C_DAT 61
static struct msm_gpio i2c_on_gpio_table[] = {
    { .gpio_cfg = GPIO_CFG(GPIO_I2C_CLK, 1, GPIO_CFG_OUTPUT,
						 GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
						 .label = "I2C_SCL" },
	{ .gpio_cfg = GPIO_CFG(GPIO_I2C_DAT, 1, GPIO_CFG_OUTPUT,
						 GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
						 .label = "I2C_SDA" },
};

static struct msm_gpio i2c_off_gpio_table[] = {
    { .gpio_cfg = GPIO_CFG(GPIO_I2C_CLK, 1, GPIO_CFG_INPUT,
						 GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
						 .label = "I2C_SCL" },
	{ .gpio_cfg = GPIO_CFG(GPIO_I2C_DAT, 1, GPIO_CFG_INPUT,
						 GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
						 .label = "I2C_SDA" },
};

void msm_set_i2c_mux(bool gpio, int *gpio_clk, int *gpio_dat)
{
	int rc;
	if (gpio) {
		rc = msm_gpios_request_enable(i2c_on_gpio_table,
										ARRAY_SIZE(i2c_on_gpio_table));
		if (rc) {
			printk(KERN_ERR "%s: unable to request/enable gpios\n", __func__);
		}
		*gpio_clk = GPIO_I2C_CLK;
		*gpio_dat = GPIO_I2C_DAT;
	} else {
		msm_gpios_disable_free(i2c_off_gpio_table,
								ARRAY_SIZE(i2c_off_gpio_table));
	}
}

static struct msm_gpio msm72k_usb_on_table[] = {
	{.gpio_cfg = GPIO_CFG(0x6f, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA0"},
	{.gpio_cfg = GPIO_CFG(0x70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA1"},
	{.gpio_cfg = GPIO_CFG(0x71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA2"},
	{.gpio_cfg = GPIO_CFG(0x72, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA3"},
	{.gpio_cfg = GPIO_CFG(0x73, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA4"},
	{.gpio_cfg = GPIO_CFG(0x74, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA5"},
	{.gpio_cfg = GPIO_CFG(0x75, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA6"},
	{.gpio_cfg = GPIO_CFG(0x76, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DATA7"},
	{.gpio_cfg = GPIO_CFG(0x77, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DIR"},
	{.gpio_cfg = GPIO_CFG(0x78, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI NEXT"},
	{.gpio_cfg = GPIO_CFG(0x79, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI STOP"},
};

static struct msm_gpio msm72k_usb_off_table[] = {
	{.gpio_cfg = GPIO_CFG(0x6f, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA0"},
	{.gpio_cfg = GPIO_CFG(0x70, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA1"},
	{.gpio_cfg = GPIO_CFG(0x71, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA2"},
	{.gpio_cfg = GPIO_CFG(0x72, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA3"},
	{.gpio_cfg = GPIO_CFG(0x73, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA4"},
	{.gpio_cfg = GPIO_CFG(0x74, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA5"},
	{.gpio_cfg = GPIO_CFG(0x75, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA6"},
	{.gpio_cfg = GPIO_CFG(0x76, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI DATA7"},
	{.gpio_cfg = GPIO_CFG(0x77, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),.label = "ULPI DIR"},
	{.gpio_cfg = GPIO_CFG(0x78, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),.label = "ULPI NEXT"},
	{.gpio_cfg = GPIO_CFG(0x79, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),.label = "ULPI STOP"},
};

struct msm_hsusb_platform_data *msm_hsusb_board_pdata = NULL;

static void msm72k_usb_connected(int enable)
{
	static bool gpios_requested = false;
	int rc;
	if (!gpios_requested) {
		rc = msm_gpios_request(msm72k_usb_on_table,
								ARRAY_SIZE(msm72k_usb_on_table));
		if (rc) {
			pr_err("%s: failed to request gpios\n", __func__);
			return;
		}
		gpios_requested = true;
	}

	if (enable) {
		if (msm_hsusb_board_pdata && msm_hsusb_board_pdata->usb_connected)
			msm_hsusb_board_pdata->usb_connected(enable);
		msm_gpios_enable(msm72k_usb_on_table, ARRAY_SIZE(msm72k_usb_on_table));
	}
	else {
		msm_gpios_disable(msm72k_usb_off_table, ARRAY_SIZE(msm72k_usb_off_table));
		if (msm_hsusb_board_pdata && msm_hsusb_board_pdata->usb_connected)
			msm_hsusb_board_pdata->usb_connected(enable);
	}
}

static void msm72k_usb_phy_reset(void) {
		if (msm_hsusb_board_pdata && msm_hsusb_board_pdata->phy_reset)
			msm_hsusb_board_pdata->phy_reset();
}

static void msm72k_usb_hw_reset(bool enable) {
		if (msm_hsusb_board_pdata && msm_hsusb_board_pdata->hw_reset)
			msm_hsusb_board_pdata->hw_reset(enable);
}

static int usb_phy_init_seq_msm72k[] = {
	0x40, 0x31,		/* High Speed TX Boost */
	0x1D, 0x0D,		/* Rising edge interrupts control register */
	0x1D, 0x10,		/* Falling edge interrupts control register */
	0x5, 0xA,		/* OTG Control register */
	-1
};

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.phy_init_seq = usb_phy_init_seq_msm72k,
	.phy_reset = msm72k_usb_phy_reset,
	.hw_reset = msm72k_usb_hw_reset,
	.usb_connected = msm72k_usb_connected,
};

static struct resource resources_hsusb[] = {
	{
		.start	= MSM_HSUSB_PHYS,
		.end	= MSM_HSUSB_PHYS + MSM_HSUSB_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_USB_HS,
		.end	= INT_USB_HS,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_hsusb = {
	.name		= "msm_hsusb",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_hsusb),
	.resource	= resources_hsusb,
	.dev		= {
		.platform_data = &msm_hsusb_pdata,
		.coherent_dma_mask	= 0xffffffff,
	},
};

struct flash_platform_data msm_nand_data = {
	.parts		= NULL,
	.nr_parts	= 0,
};

static struct resource resources_nand[] = {
	[0] = {
		.start	= 7,
		.end	= 7,
		.flags	= IORESOURCE_DMA,
	},
};

struct platform_device msm_device_nand = {
	.name		= "msm_nand",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_nand),
	.resource	= resources_nand,
	.dev		= {
		.platform_data	= &msm_nand_data,
	},
};

static struct resource resources_sdc1[] = {
	{
		.start	= MSM_SDC1_PHYS,
		.end	= MSM_SDC1_PHYS + MSM_SDC1_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_SDC1_0,
		.end	= INT_SDC1_0,
		.flags	= IORESOURCE_IRQ,
		.name	= "cmd_irq",
	},
	{
		.start	= INT_SDC1_1,
		.end	= INT_SDC1_1,
		.flags	= IORESOURCE_IRQ,
		.name	= "pio_irq",
	},
	{
		.flags	= IORESOURCE_IRQ | IORESOURCE_DISABLED,
		.name	= "status_irq"
	},
	{
		.start	= DMOV_SDC1_CHAN,
		.end	= DMOV_SDC1_CHAN,
		.flags	= IORESOURCE_DMA,
	},
};

static struct resource resources_sdc2[] = {
	{
		.start	= MSM_SDC2_PHYS,
		.end	= MSM_SDC2_PHYS + MSM_SDC2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_SDC2_0,
		.end	= INT_SDC2_0,
		.flags	= IORESOURCE_IRQ,
		.name	= "cmd_irq",
	},
		{
		.start	= INT_SDC2_1,
		.end	= INT_SDC2_1,
		.flags	= IORESOURCE_IRQ,
		.name	= "pio_irq",
	},
	{
		.flags	= IORESOURCE_IRQ | IORESOURCE_DISABLED,
		.name	= "status_irq"
	},
	{
		.start	= DMOV_SDC2_CHAN,
		.end	= DMOV_SDC2_CHAN,
		.flags	= IORESOURCE_DMA,
	},
};

static struct resource resources_sdc3[] = {
	{
		.start	= MSM_SDC3_PHYS,
		.end	= MSM_SDC3_PHYS + MSM_SDC3_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_SDC3_0,
		.end	= INT_SDC3_0,
		.flags	= IORESOURCE_IRQ,
		.name	= "cmd_irq",
	},
		{
		.start	= INT_SDC3_1,
		.end	= INT_SDC3_1,
		.flags	= IORESOURCE_IRQ,
		.name	= "pio_irq",
	},
	{
		.flags	= IORESOURCE_IRQ | IORESOURCE_DISABLED,
		.name	= "status_irq"
	},
	{
		.start	= DMOV_SDC3_CHAN,
		.end	= DMOV_SDC3_CHAN,
		.flags	= IORESOURCE_DMA,
	},
};

static struct resource resources_sdc4[] = {
	{
		.start	= MSM_SDC4_PHYS,
		.end	= MSM_SDC4_PHYS + MSM_SDC4_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_SDC4_0,
		.end	= INT_SDC4_0,
		.flags	= IORESOURCE_IRQ,
		.name	= "cmd_irq",
	},
		{
		.start	= INT_SDC4_1,
		.end	= INT_SDC4_1,
		.flags	= IORESOURCE_IRQ,
		.name	= "pio_irq",
	},
	{
		.flags	= IORESOURCE_IRQ | IORESOURCE_DISABLED,
		.name	= "status_irq"
	},
	{
		.start	= DMOV_SDC4_CHAN,
		.end	= DMOV_SDC4_CHAN,
		.flags	= IORESOURCE_DMA,
	},
};

struct platform_device msm_device_sdc1 = {
	.name		= "msm_sdcc",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(resources_sdc1),
	.resource	= resources_sdc1,
	.dev		= {
		.coherent_dma_mask	= 0xffffffff,
	},
};

struct platform_device msm_device_sdc2 = {
	.name		= "msm_sdcc",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(resources_sdc2),
	.resource	= resources_sdc2,
	.dev		= {
		.coherent_dma_mask	= 0xffffffff,
	},
};

struct platform_device msm_device_sdc3 = {
	.name		= "msm_sdcc",
	.id		= 3,
	.num_resources	= ARRAY_SIZE(resources_sdc3),
	.resource	= resources_sdc3,
	.dev		= {
		.coherent_dma_mask	= 0xffffffff,
	},
};

struct platform_device msm_device_sdc4 = {
	.name		= "msm_sdcc",
	.id		= 4,
	.num_resources	= ARRAY_SIZE(resources_sdc4),
	.resource	= resources_sdc4,
	.dev		= {
		.coherent_dma_mask	= 0xffffffff,
	},
};

static struct platform_device *msm_sdcc_devices[] __initdata = {
	&msm_device_sdc1,
	&msm_device_sdc2,
	&msm_device_sdc3,
	&msm_device_sdc4,
};

int __init msm_add_sdcc(unsigned int controller, struct msm_mmc_platform_data *plat,
			unsigned int stat_irq, unsigned long stat_irq_flags)
{
	struct platform_device	*pdev;
	struct resource *res;

	if (controller < 1 || controller > 4)
		return -EINVAL;

	pdev = msm_sdcc_devices[controller-1];
	pdev->dev.platform_data = plat;

	res = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "status_irq");
	if (!res)
		return -EINVAL;
	else if (stat_irq) {
		res->start = res->end = stat_irq;
		res->flags &= ~IORESOURCE_DISABLED;
		res->flags |= stat_irq_flags;
	}

	return platform_device_register(pdev);
}

void msm_delete_sdcc(unsigned int controller) {
	struct platform_device	*pdev;
	if (controller < 1 || controller > 4)
		return;
	pdev = msm_sdcc_devices[controller-1];
	platform_device_unregister(pdev);
}

static struct resource resources_mddi0[] = {
	{
		.start	= MSM_PMDH_PHYS,
		.end	= MSM_PMDH_PHYS + MSM_PMDH_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_MDDI_PRI,
		.end	= INT_MDDI_PRI,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource resources_mddi1[] = {
	{
		.start	= MSM_EMDH_PHYS,
		.end	= MSM_EMDH_PHYS + MSM_EMDH_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_MDDI_EXT,
		.end	= INT_MDDI_EXT,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_mddi0 = {
	.name = "msm_mddi",
	.id = 0,
	.num_resources = ARRAY_SIZE(resources_mddi0),
	.resource = resources_mddi0,
	.dev = {
		.coherent_dma_mask      = 0xffffffff,
	},
};

struct platform_device msm_device_mddi1 = {
	.name = "msm_mddi",
	.id = 1,
	.num_resources = ARRAY_SIZE(resources_mddi1),
	.resource = resources_mddi1,
	.dev = {
		.coherent_dma_mask      = 0xffffffff,
	},
};

static struct resource resources_mdp[] = {
	{
		.start	= MSM_MDP_PHYS,
		.end	= MSM_MDP_PHYS + MSM_MDP_SIZE - 1,
		.name	= "mdp",
		.flags	= IORESOURCE_MEM
	},
	{
		.start	= INT_MDP,
		.end	= INT_MDP,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_mdp = {
	.name = "msm_mdp",
	.id = 0,
	.num_resources = ARRAY_SIZE(resources_mdp),
	.resource = resources_mdp,
};

static struct resource resources_tssc[] = {
	{
		.start	= MSM_TSSC_PHYS,
		.end	= MSM_TSSC_PHYS + MSM_TSSC_SIZE - 1,
		.name	= "tssc",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_TCHSCRN1,
		.end	= INT_TCHSCRN1,
		.name	= "tssc1",
		.flags	= IORESOURCE_IRQ | IRQF_TRIGGER_RISING,
	},
	{
		.start	= INT_TCHSCRN2,
		.end	= INT_TCHSCRN2,
		.name	= "tssc2",
		.flags	= IORESOURCE_IRQ | IRQF_TRIGGER_RISING,
	},
};

struct platform_device msm_device_touchscreen = {
	.name = "msm_touchscreen",
	.id = 0,
	.num_resources = ARRAY_SIZE(resources_tssc),
	.resource = resources_tssc,
};

struct clk msm_clocks_7x01a[] = {
	CLK_IMPL("adm_clk",	ADM_CLK,	NULL, 0),
	CLK_IMPL("adsp_clk",	ADSP_CLK,	NULL, 0),
	CLK_IMPL("ebi1_clk",	EBI1_CLK,	NULL, CLK_MIN),
	CLK_IMPL("ebi2_clk",	EBI2_CLK,	NULL, 0),
	CLK_IMPL("ecodec_clk",	ECODEC_CLK,	NULL, 0),
	CLK_IMPL("mddi_clk",	EMDH_CLK,	&msm_device_mddi1.dev, OFF),
	CLK_IMPL("gp_clk",	GP_CLK,		NULL, 0),
	CLK_IMPL("grp_clk",	GRP_3D_CLK,	NULL, OFF),
	CLK_IMPL("i2c_clk",	I2C_CLK,	&msm_device_i2c.dev, 0),
	CLK_IMPL("icodec_rx_clk",	ICODEC_RX_CLK,	NULL, 0),
	CLK_IMPL("icodec_tx_clk",	ICODEC_TX_CLK,	NULL, 0),
	CLK_IMPL("imem_clk",	IMEM_CLK,	NULL, OFF),
	CLK_IMPL("mdc_clk",	MDC_CLK,	NULL, 0),
	CLK_IMPL("mdp_clk",	MDP_CLK,	&msm_device_mdp.dev, OFF),
	CLK_IMPL("pbus_clk",	PBUS_CLK,	NULL, 0),
	CLK_IMPL("pcm_clk",	PCM_CLK,	NULL, 0),
	CLK_IMPL("mddi_clk",	PMDH_CLK,	&msm_device_mddi0.dev, OFF | CLK_MINMAX),
	CLK_IMPL("sdac_clk",	SDAC_CLK,	NULL, OFF),
	CLK_IMPL("sdc_clk",	SDC1_CLK,	&msm_device_sdc1.dev, OFF),
	CLK_IMPL("sdc_pclk",	SDC1_P_CLK,	&msm_device_sdc1.dev, OFF),
	CLK_IMPL("sdc_clk",	SDC2_CLK,	&msm_device_sdc2.dev, OFF),
	CLK_IMPL("sdc_pclk",	SDC2_P_CLK,	&msm_device_sdc2.dev, OFF),
	CLK_IMPL("sdc_clk",	SDC3_CLK,	&msm_device_sdc3.dev, OFF),
	CLK_IMPL("sdc_pclk",	SDC3_P_CLK,	&msm_device_sdc3.dev, OFF),
	CLK_IMPL("sdc_clk",	SDC4_CLK,	&msm_device_sdc4.dev, OFF),
	CLK_IMPL("sdc_pclk",	SDC4_P_CLK,	&msm_device_sdc4.dev, OFF),
	CLK_IMPL("tsif_clk",	TSIF_CLK,	NULL, 0),
	CLK_IMPL("tsif_ref_clk",	TSIF_REF_CLK,	NULL, 0),
	CLK_IMPL("tv_dac_clk",	TV_DAC_CLK,	NULL, 0),
	CLK_IMPL("tv_enc_clk",	TV_ENC_CLK,	NULL, 0),
	CLK_IMPL("uart_clk",	UART1_CLK,	&msm_device_uart1.dev, OFF),
	CLK_IMPL("uart_clk",	UART2_CLK,	&msm_device_uart2.dev, OFF),
	CLK_IMPL("uart_clk",	UART3_CLK,	&msm_device_uart3.dev, OFF),
	CLK_IMPL("uartdm_clk",	UART1DM_CLK,	&msm_device_uart_dm1.dev, OFF),
	CLK_IMPL("uartdm_clk",	UART2DM_CLK,	&msm_device_uart_dm2.dev, OFF),
	CLK_IMPL("usb_hs_clk",	USB_HS_CLK,	&msm_device_hsusb.dev, OFF),
	CLK_IMPL("usb_hs_pclk",	USB_HS_P_CLK,	&msm_device_hsusb.dev, OFF),
	CLK_IMPL("usb_otg_clk",	USB_OTG_CLK,	NULL, 0),
	CLK_IMPL("vdc_clk",	VDC_CLK,	NULL, OFF | CLK_MINMAX),
	CLK_IMPL("vfe_clk",	VFE_CLK,	NULL, OFF),
	CLK_IMPL("vfe_mdc_clk",	VFE_MDC_CLK,	NULL, OFF),
};

unsigned msm_num_clocks_7x01a = ARRAY_SIZE(msm_clocks_7x01a);
