/* arch/arm/mach-msm/include/mach/board.h
 *
 * Copyright (C) 2007 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
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

#ifndef __ASM_ARCH_MSM_BOARD_H
#define __ASM_ARCH_MSM_BOARD_H

#include <linux/types.h>
#include <mach/mmc.h>

struct msm_pmem_setting {
	resource_size_t pmem_start;
	resource_size_t pmem_size;
	resource_size_t pmem_adsp_start;
	resource_size_t pmem_adsp_size;
	resource_size_t pmem_gpu0_start;
	resource_size_t pmem_gpu0_size;
	resource_size_t pmem_gpu1_start;
	resource_size_t pmem_gpu1_size;
	resource_size_t pmem_camera_start;
	resource_size_t pmem_camera_size;
	resource_size_t ram_console_start;
	resource_size_t ram_console_size;
};

/* platform device data structures */
struct msm_acpu_clock_platform_data
{
	uint32_t acpu_switch_time_us;
	uint32_t max_speed_delta_khz;
	uint32_t vdd_switch_time_us;
	unsigned long mpll_khz;
	unsigned long power_collapse_khz;
	unsigned long wait_for_irq_khz;
	unsigned int max_axi_khz;
};

struct msm_camera_io_ext {
	uint32_t mdcphy;
	uint32_t mdcsz;
	uint32_t appphy;
	uint32_t appsz;
	unsigned long camifpadphy;
	unsigned long camifpadsz;
};

struct msm_camera_device_platform_data {
	void (*camera_gpio_on) (void);
	void (*camera_gpio_off)(void);
	struct msm_camera_io_ext ioext;
};

struct msm_camera_legacy_device_platform_data {
	int sensor_reset;
	int sensor_pwd;
	int vcm_pwd;
	void (*config_gpio_on) (void);
	void (*config_gpio_off)(void);
};

struct msm_camera_sensor_info {
	const char *sensor_name;
	int sensor_reset;
	int sensor_pwd;
	int vcm_pwd;
	int mclk;
	int num_flash_levels;
	int (*camera_flash)(int level);
	int need_suspend;
	struct msm_camera_device_platform_data *pdata;
	struct resource *resource;
	uint8_t num_resources;
};

struct clk;

/* common init routines for use by arch/arm/mach-msm/board-*.c */

void __init msm_add_devices(void);
void __init msm_map_common_io(void);
void __init msm_init_irq(void);
void __init msm_clock_init(struct clk *clock_tbl, unsigned num_clocks);
void __init msm_acpu_clock_init(struct msm_acpu_clock_platform_data *);
int __init msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat,
			unsigned int stat_irq, unsigned long stat_irq_flags);

#ifdef CONFIG_ANDROID_PMEM
void __init msm_add_mem_devices(struct msm_pmem_setting *setting);
#else
static inline void __init msm_add_mem_devices(struct msm_pmem_setting *setting) {
}
#endif

#ifdef CONFIG_USB_MSM_72K
void msm_hsusb_set_vbus_state(int online);
#else
static inline void msm_hsusb_set_vbus_state(int online) {}
#endif

#endif
