/* linux/include/asm-arm/arch-msm/msm_smd.h
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

#ifndef __ASM_ARCH_MSM_SMD_H
#define __ASM_ARCH_MSM_SMD_H

#include <linux/types.h>

typedef struct smd_channel smd_channel_t;

extern int (*msm_check_for_modem_crash)(void);

/* warning: notify() may be called before open returns */
int smd_open(const char *name, smd_channel_t **ch, void *priv,
	     void (*notify)(void *priv, unsigned event));

#define SMD_EVENT_DATA 1
#define SMD_EVENT_OPEN 2
#define SMD_EVENT_CLOSE 3

int smd_close(smd_channel_t *ch);

/* passing a null pointer for data reads and discards */
int smd_read(smd_channel_t *ch, void *data, int len);

/* Write to stream channels may do a partial write and return
** the length actually written.
** Write to packet channels will never do a partial write --
** it will return the requested length written or an error.
*/
int smd_write(smd_channel_t *ch, const void *data, int len);
int smd_write_atomic(smd_channel_t *ch, const void *data, int len);

int smd_write_avail(smd_channel_t *ch);
int smd_read_avail(smd_channel_t *ch);

/* Returns the total size of the current packet being read.
** Returns 0 if no packets available or a stream channel.
*/
int smd_cur_packet_size(smd_channel_t *ch);

/* used for tty unthrottling and the like -- causes the notify()
** callback to be called from the same lock context as is used
** when it is called from channel updates
*/
void smd_kick(smd_channel_t *ch);


#if 0
/* these are interruptable waits which will block you until the specified
** number of bytes are readable or writable.
*/
int smd_wait_until_readable(smd_channel_t *ch, int bytes);
int smd_wait_until_writable(smd_channel_t *ch, int bytes);
#endif

void smem_semaphore_down(void* address, char marker);
void smem_semaphore_up(void* address, char marker);

typedef enum {
	SMD_PORT_DS = 0,
	SMD_PORT_DIAG,
	SMD_PORT_RPC_CALL,
	SMD_PORT_RPC_REPLY,
	SMD_PORT_BT,
	SMD_PORT_CONTROL,
	SMD_PORT_MEMCPY_SPARE1,
	SMD_PORT_DATA1,
	SMD_PORT_DATA2,
	SMD_PORT_DATA3,
	SMD_PORT_DATA4,
	SMD_PORT_DATA5,
	SMD_PORT_DATA6,
	SMD_PORT_DATA7,
	SMD_PORT_DATA8,
	SMD_PORT_DATA9,
	SMD_PORT_DATA10,
	SMD_PORT_DATA11,
	SMD_PORT_DATA12,
	SMD_PORT_DATA13,
	SMD_PORT_DATA14,
	SMD_PORT_DATA15,
	SMD_PORT_DATA16,
	SMD_PORT_DATA17,
	SMD_PORT_DATA18,
	SMD_PORT_DATA19,
	SMD_PORT_DATA20,
	SMD_PORT_GPS_NMEA,
	SMD_PORT_BRIDGE_1,
	SMD_PORT_BRIDGE_2,
	SMD_PORT_BRIDGE_3,
	SMD_PORT_BRIDGE_4,
	SMD_PORT_BRIDGE_5,
	SMD_PORT_LOOPBACK,
	SMD_PORT_CS_APPS_MODEM,
	SMD_PORT_CS_APPS_DSP,
	SMD_PORT_CS_MODEM_DSP,
	SMD_NUM_PORTS,
} smd_port_id_type;

struct smd_tty_channel_desc {
	int id;
	const char *name;
};

int smd_set_channel_list(const struct smd_tty_channel_desc *, int len);

/******* Platform Data setup **********/
enum amss_id {
	AMSS_AUDMGR_PROG,
	AMSS_AUDMGR_VERS,
	AMSS_AUDMGR_CB_PROG,
	AMSS_AUDMGR_CB_VERS,
	AMSS_DOG_KEEPALIVE_PROG,
	AMSS_DOG_KEEPALIVE_VERS,
	AMSS_DOG_KEEPALIVE_BEACON,
	AMSS_PM_LIBVERS,
	AMSS_RTC_PROG,
	AMSS_RTC_VERS,
	AMSS_SND_PROG,
	AMSS_SND_VERS,
	AMSS_SND_SET_DEVICE_PROC,
	AMSS_SND_SET_VOLUME_PROC,
	AMSS_TIME_REMOTE_MTOA_VERS,
	AMSS_TIME_TOD_SET_APPS_BASES,
};

struct amss_value {
	enum amss_id id;
	uint32_t value;
};

struct msm_early_server {
	uint32_t cid;
	uint32_t pid;
	uint32_t prog;
	uint32_t vers;
};

struct msm_smd_platform_data {
		struct amss_value *amss_values;
		int n_amss_values;
		struct msm_early_server *early_servers;
		int n_early_servers;
		bool use_v2_alloc_elm;
};

extern bool amss_get_id(enum amss_id, uint32_t* out);

#endif
