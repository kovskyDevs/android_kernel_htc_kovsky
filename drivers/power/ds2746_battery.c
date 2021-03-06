/*
 * Driver for batteries with DS2746 chips inside.
 *
 * Copyright � 2009 Matthew Kern
 * 	       2007 Anton Vorontsov
 *	       2004-2007 Matt Reimer
 *	       2004 Szabolcs Gyurko
 *
 * Use consistent with the GNU GPL is permitted,
 * provided that this copyright notice is
 * preserved in its entirety in all copies and derived works.
 *
 * Author:  Matthew Kern <pyrophobicman@gmail.com
 * 	    January 2009
 *
 * 	    Anton Vorontsov <cbou@mail.ru>
 *	    February 2007
 *
 *	    Matt Reimer <mreimer@vpop.net>
 *	    April 2004, 2005, 2007
 *
 *	    Szabolcs Gyurko <szabolcs.gyurko@tlt.hu>
 *	    September 2004
 *
 *      Martin Johnson <m.j.johnson@massey.ac.nz>
 *      February 2009  - Simplified for HTC Kaiser
 *
 *	Stefan Seidel <kaiser@stefanseidel.info>
 *	March 2009 - Correct battery charge pctg. calculation
 *
 *      Carsten Wilhelm <carsten@wilhelm-net.de>
 *      May 2010 - Changed implementation for DEFAULT (aka Xperia X1)
 *
 *	Mathew McBride <matt@mcbridematt.dhs.org>
 *	August 2010 - Made standalone
 *
 * 	Alexander Tarasikov <alexander.tarasikov@gmail.com>
 * 	October 2010 - made generic
*/

/* References:
	ds2760 battery driver
	ds2782 battery driver
	ds2784 battery driver in Android kernel tree
 */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/pm.h>
#include <linux/power_supply.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/ds2746_battery.h>
#include <asm/gpio.h>

#include <linux/android_alarm.h>
#include <linux/wakelock.h>

int TEMP_MAP_1000K_100_4360[] =
{
	0, 30, 31, 32, 34, 35, 36, 38, 39, 40,
	42, 44, 45, 47, 49, 51, 53, 55, 57, 60,
	62, 64, 67, 70, 73, 76, 79, 82, 86, 89,
	93, 97, 101, 106, 111, 115, 120, 126, 131, 137,
	143, 150, 156, 163, 171, 178, 187, 195, 204, 213,
	223, 233, 244, 255, 267, 279, 292, 306, 320, 334,
	350, 365, 382, 399, 418, 436, 456, 476, 497, 519,
	542, 566, 590, 615, 641, 668, 695, 723, 752, 782,
	812, 843, 2047,
};

int *TEMP_MAP = TEMP_MAP_1000K_100_4360;
#define TEMP_MAX 70
#define TEMP_MIN -11
#define TEMP_NUM 83

/* definitions for registers we care about. */
#define DS2746_DATA_SIZE		0x12

#define DS2746_STATUS_REG		0x01
#define DS2746_AUX0_MSB			0x08
#define DS2746_AUX0_LSB			0x09
#define DS2746_AUX1_MSB			0x0a
#define DS2746_AUX1_LSB			0x0b
#define DS2746_VOLTAGE_MSB		0x0c
#define DS2746_VOLTAGE_LSB		0x0d
#define DS2746_CURRENT_MSB		0x0e
#define DS2746_CURRENT_LSB		0x0f
#define DS2746_CURRENT_ACCUM_MSB	0x10
#define DS2746_CURRENT_ACCUM_LSB	0x11
#define DS2746_OFFSET_BIAS              0x61
#define DS2746_ACCUM_BIAS               0x62

#define DEFAULT_RSNS			1500	// in mOHM. From the maths, this should be correct
#define DEFAULT_BATTERY_RATING		1500	// capacity of standard DEFAULT battery 1500mAh
#define DEFAULT_HIGH_VOLTAGE		4196
#define DEFAULT_LOW_VOLTAGE		3500

#define DS2746_CURRENT_ACCUM_RES	690	// resolution of ACCUM-register in uVh * 100 per bit
#define DS2746_VOLTAGE_RES		2440	// resolution of voltage register multiplied by 1000
#define DS2746_NEAR_END_CHARGE		 100
#define DS2746_MINI_CURRENT_FOR_CHARGE  30	// Minimum batt_current to consider battery is charging
#define DS2746_MAX_ACCUM_VALUE 4000 // Max value for ACR (correct invalid values)
#define DS2746_TOO_HIGH_ACCUM_VALUE 5000 // Too high value for ACR
#define DS2746_MIN_ACCUM_VALUE   10 // Minimum value of acr register when not commpletly empty.

#define DS2746_STABLE_RANGE		 300  // Range for 3 last bat_curent to consider it's stable
#define DS2746_5PERCENT_VOLTAGE	 120  // How much more than low_voltage is 15%
#define DS2746_ACCUM_BIAS_DEFAULT	   0  // unit = 1.56mV/Rsns


#define FAST_POLL (30 * 1000)
#define SLOW_POLL (2 * 60 * 1000)

struct i2c_client *pclient = 0;

static unsigned int battery_capacity = DEFAULT_BATTERY_RATING;
static unsigned int current_accum_capacity = 500;

module_param(battery_capacity, int, 0644);
MODULE_PARM_DESC(battery_capacity, "Estimated battery capacity in mAh");

#define DBG(fmt, x...) pr_debug(fmt, ##x)

struct ds2746_info {
	u32 batt_id;		/* Battery ID from ADC */
	u32 batt_vol;		/* Battery voltage from ADC */
	u32 batt_temp;		/* Battery Temperature (C) from formula and ADC */
	int aver_batt_current; /* Information about batt current send to Android */
	int batt_current;	/* Battery current from ADC */
	int batt_current_1;
	int batt_current_2;
	int batt_current_3;
	int batt_current_4;
	int batt_current_5;
	int batt_history_nb;
	int last_am_i_supplied;
	
	u32 level;		/* formula */
	u32 charging_source;	/* 0: no cable, 1:usb, 2:AC */
	bool charging_enabled;	/* 0: Disable, 1: Enable */
	u32 full_bat;		/* Full capacity of battery (mAh) */
	u32 last_s_value;	/* last CURRENT_ACCUM register value */
	
	int temp_adc;
	int last_temp_adc;
	int last_temp_index;
	
	struct ds2746_platform_data bat_pdata;
	struct power_supply *bat;	/* Hold the supply struct so it can be passed along */
};

static struct ds2746_info *bi;

/** TODO: Should all of this live in the battery_info struct instead? */
/* Mutexes */
static DEFINE_MUTEX(bat_lock);
static struct delayed_work bat_work;
static struct wake_lock chrg_lock;
struct mutex work_lock;

/* Polling  */
struct workqueue_struct *monitor_wqueue;	/* Work queue used to ping the battery */

/* Polling periods - fast if on wall, slow if on battery */
/* Power management API stuff */
static enum power_supply_property ds2746_bat_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TECHNOLOGY
};

static int
ds2746_bat_get_property(struct power_supply *bat_ps,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (bi->charging_enabled) {
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
		} else {
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
		}
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = bi->level;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = bi->batt_temp;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = (bi->batt_vol * 1000);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = bi->aver_batt_current;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void ds2746_ext_power_changed(struct power_supply *psy)
{
	cancel_delayed_work(&bat_work);
	queue_delayed_work(monitor_wqueue, &bat_work, 0);
}

static struct power_supply bat_ps = {
	.name = DS2746_NAME,
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = ds2746_bat_properties,
	.num_properties = ARRAY_SIZE(ds2746_bat_properties),
	.get_property = ds2746_bat_get_property,
	.external_power_changed = ds2746_ext_power_changed,
};

/* -----------------------------------------------------------
 * i2c utilitiy functions
 */
#define I2C_READ_RETRY_TIMES 10
#define I2C_WRITE_RETRY_TIMES 10

static int i2c_read(int r)
{
	int retry;
	int rc;
	unsigned char i2c_msg[1];
	unsigned char i2c_data[2];
	
	
	struct i2c_msg msgs[] = {
		{
		 .addr = 0,
		 .flags = 0,
		 .len = 1,
		 .buf = i2c_msg,
		 },
		{
		 .addr = 0,
		 .flags = I2C_M_RD,
		 .len = 2,
		 .buf = i2c_data,
		 }
	};

	i2c_msg[0]= r & 0xff;
	
	if (!pclient) {
		pr_err("%s: pclient is null\n", __func__);
		return -EIO;
	}

	msgs[0].addr = msgs[1].addr = pclient->addr;

	for (retry = 0; retry <= I2C_READ_RETRY_TIMES; retry++) {
		rc = i2c_transfer(pclient->adapter, msgs, 2);
		if (rc == 2) {
			rc = 0;
			goto exit;
		}
		msleep(10);
		pr_err("microp_ng, i2c read retry\n");
	}

	if (retry >= I2C_WRITE_RETRY_TIMES) {
		dev_err(&pclient->dev, "i2c_read_block retry over %d\n",
			I2C_READ_RETRY_TIMES);
		rc = -EIO;
	}

exit:
	return ((int)i2c_data[0])&0xff;
}

static int i2c_read_signed(int r)
{
	int data = i2c_read(r);
	
	if((data & 0x80) == 0)
		return data;
	else
		return -(data & 0x7F);
}


static void i2c_write(int r, int v)
{
	unsigned char i2c_msg[3];
	int ret;
	i2c_msg[0] = r;
	i2c_msg[1] = v & 0xFF;
	ret = i2c_master_send(pclient, i2c_msg, 2);
	if(ret==-EAGAIN){
	  msleep(10);
	  printk("ds2746: i2c bus busy; Retrying write\n");
	  ret = i2c_master_send(pclient, i2c_msg, 2);
	}
}

static void i2c_write_signed(int r, int v)
{
	unsigned char i2c_msg[2];
	int ret;
	i2c_msg[0] = r;
	if(v>=0)
		i2c_msg[1] = min(v, 0x7F);
	else
		i2c_msg[1] = 0x80 | min(-v, 0x7F);
	ret = i2c_master_send(pclient, i2c_msg, 2);
	if(ret==-EAGAIN){
	  msleep(1);
	  printk("ds2746: i2c bus busy; Retrying write\n");
	  ret = i2c_master_send(pclient, i2c_msg, 2);
	}
}

static int reading2capacity(int r)
{
	return (DS2746_CURRENT_ACCUM_RES * r) / (bi->bat_pdata.resistance);
}

static int ds2746_battery_read_temp(struct ds2746_info *b)
{
	int cur_idx = b->last_temp_index;
	int search_dir;
	int temp;
	
	if (b->last_temp_adc > b->temp_adc) {
		search_dir = -1;
	} else {
		search_dir = 1;
	}

	while (cur_idx >= 0 && cur_idx < TEMP_NUM-1) {
		int temp_min = TEMP_MAP[cur_idx];
		int temp_max = TEMP_MAP[cur_idx + 1];

		if (temp_max > b->temp_adc && temp_min <= b->temp_adc) {
			b->last_temp_index = cur_idx;
			b->last_temp_adc = b->temp_adc;
			temp = (TEMP_MAX - cur_idx) * 10;;
			if(temp > 600){
			  printk("%s: Temp was calculated to %d. Reduced to 60\n", __func__, temp/10);
			  temp=600;
			}
			return temp;
		}

		cur_idx += search_dir;
	}

	return (TEMP_MIN-1) * 10;
}

signed short set_accum_value(int aux0)
{
	signed short s;
	//int aux0r, aux1r;

	/* Get our correctors value */
	//aux0r = i2c_read_signed(DS2746_ACCUM_BIAS);
	//aux1r = i2c_read_signed(DS2746_OFFSET_BIAS);

	/* Set accum bias register to 0 */
	//i2c_write_signed(DS2746_ACCUM_BIAS, 0);

	/* Write accum value */
	i2c_write(DS2746_CURRENT_ACCUM_LSB, aux0 & 0xFF);
	i2c_write(DS2746_CURRENT_ACCUM_MSB, aux0 >> 8);

	/* Reset our correctors value (we want to keep ours, maybe only accum value is too high */
	//i2c_write_signed(DS2746_ACCUM_BIAS, aux0r);
	//i2c_write_signed(DS2746_OFFSET_BIAS, aux1r);

	/* Read and info to be sure */
	s = i2c_read(DS2746_CURRENT_ACCUM_LSB);
	s |= i2c_read(DS2746_CURRENT_ACCUM_MSB) << 8;

	return s;
}

static int ds2746_battery_read_status(struct ds2746_info *b)
{
	signed short volt;
	signed short cur;
	unsigned short acr;
	int aux0;
	int aux0r, aux1r;
	int aver_batt_current;
	int max_current;
	int all_positive;
	int charge_ended = 0;
	int voltage_diff;
	int r_ain0;

	if (!pclient) {
		pr_err("client is null\n");
		return -ENODEV;
	}

	/* Get voltage value */
	volt = i2c_read(DS2746_VOLTAGE_LSB);
	volt |= i2c_read(DS2746_VOLTAGE_MSB) << 8;
	b->batt_vol = ((volt >> 4) * DS2746_VOLTAGE_RES) / 1000;		

	/* Get and current value and actualize history */
	cur = i2c_read(DS2746_CURRENT_LSB);
	cur |= i2c_read(DS2746_CURRENT_MSB) << 8;
	cur >>= 2;
	b->batt_current_5 = b->batt_current_4;
	b->batt_current_4 = b->batt_current_3;
	b->batt_current_3 = b->batt_current_2;
	b->batt_current_2 = b->batt_current_1;
	b->batt_current_1 = b->batt_current;
	b->batt_current = (cur * DS2746_CURRENT_ACCUM_RES) / (bi->bat_pdata.resistance);
	b->batt_history_nb++;
	max_current = max(abs(b->batt_current_5), max(abs(b->batt_current_4), max(abs(b->batt_current_3),
    								max(abs(b->batt_current_2), max(abs(b->batt_current_1), abs(b->batt_current))))));

	/* Get accum value */
	acr = i2c_read(DS2746_CURRENT_ACCUM_LSB);
	acr |= i2c_read(DS2746_CURRENT_ACCUM_MSB) << 8;

	/* Correct if necessary accum value */
	if (acr > DS2746_MAX_ACCUM_VALUE)
		{
			pr_info("ds2746: Correcting too high ACR value from %d to %d.\n", acr, DS2746_MAX_ACCUM_VALUE);
			acr = set_accum_value(DS2746_MAX_ACCUM_VALUE);
		}

	/* Get BIAS values */
	aux0r = i2c_read_signed(DS2746_ACCUM_BIAS);
	aux1r = i2c_read_signed(DS2746_OFFSET_BIAS);

	aver_batt_current = (b->batt_current + b->batt_current_1 +  b->batt_current_2 +
											 + b->batt_current_3 + b->batt_current_4 + b->batt_current_5) / 6;
	all_positive = (b->batt_current >= 0) && (b->batt_current_1 >= 0) && (b->batt_current_2 >= 0) &&
		(b->batt_current_3 >= 0) && (b->batt_current_4 >= 0) && (b->batt_current_5 >= 0);

	/*pr_info("ds2746 debug : %d mV level %d, accum %u / %u current %d (%d/%d/%d/%d/%d/%d)\n",
				 b->batt_vol, bi->level, acr, current_accum_capacity, aver_batt_current,
				 b->batt_current, b->batt_current_1,  b->batt_current_2,
				 b->batt_current_3, b->batt_current_4, b->batt_current_5);*/

	/* Wait for some history before using average */
	if(b->batt_history_nb > 8)
		{
			/* LOW VOLTAGE */

			/* if battery voltage is < 3.5V and depleting, we assume it's almost empty! */
			if (b->batt_vol < bi->bat_pdata.low_voltage+DS2746_5PERCENT_VOLTAGE &&
					aver_batt_current < 0) {
				/* use approximate formula: 3.5V=15%, 3.35V=0% correction-factor is */
				/* (capacity * 0.05) / (3500 - 3350)  or (capacity*5/(100 * 150) */
				if (b->batt_vol <= bi->bat_pdata.low_voltage + 30)
					voltage_diff = 30;
				else
					voltage_diff = b->batt_vol - bi->bat_pdata.low_voltage;
				aux0 = (voltage_diff * current_accum_capacity * 15) /	(100 * DS2746_5PERCENT_VOLTAGE);

				/* Ponderate value with acr actual value */
				aux0 = (2*acr + aux0) / 3;

				if (abs(aux0 - acr) > 1 && aux0 > 1 && acr > 1) {
					pr_info("ds2746: LOW VOLTAGE (%d) ACR is %d, should be %d\n", b->batt_vol, acr, aux0);
					acr = set_accum_value(aux0);
				}
			}

			/* MEDIUM VOLTAGE */
			/* if battery voltage is > 3.5V and depleting, should not be less than 15% */
			if (b->batt_vol > bi->bat_pdata.low_voltage+DS2746_5PERCENT_VOLTAGE &&
					aver_batt_current < 0 && (acr*100)/current_accum_capacity < 15) {

				/* Voltage is not low enought for a credible <15% level */
				aux0 = (current_accum_capacity*15)/100;

				/* Ponderate value with s actual value */
				aux0 = (2*acr + aux0) / 3;

				if (abs(aux0 - acr) > 1 && aux0 > 1 && acr > 1) {
					pr_info("ds2746: MEDIUM VOLTAGE (%d / %d) ACR is too low, updated %d\n",
								 b->batt_vol, current_accum_capacity, aux0);
					acr = set_accum_value(aux0);
				}
			}

			/* HIGH VOLTAGE */
			if (b->batt_vol >= bi->bat_pdata.high_voltage &&
					max_current <  DS2746_MINI_CURRENT_FOR_CHARGE*2 &&
					abs(aver_batt_current) < DS2746_MINI_CURRENT_FOR_CHARGE) {

				/* Charge ended */
				charge_ended = 1;

				/* Set accum to max if superior, dont allow grow forever */
				if(acr > current_accum_capacity) {
					acr = current_accum_capacity;
					acr = set_accum_value(acr);
				}
				else if(acr < current_accum_capacity) {
					current_accum_capacity = acr;
				}
			}
			else if (b->batt_vol >= bi->bat_pdata.high_voltage && all_positive &&
							 abs(aver_batt_current) < DS2746_NEAR_END_CHARGE) {

				/* Current is corrected 0 */
				aver_batt_current = 0;

				/* Set accum to max-1 if superior because we are near end charge */
				if(acr > current_accum_capacity-1) {
					acr = current_accum_capacity;
					acr = set_accum_value(acr);
					current_accum_capacity++;
				}
			}
			else {
				/* No high voltage + low current : charge not ended check if "fuller" */
				if (acr >= current_accum_capacity) {
					/* if the battery is "fuller" than expected update our expectations */
					current_accum_capacity = acr + 1;
				}
			}
		}
	b->aver_batt_current = aver_batt_current;

	b->last_s_value=acr;
	battery_capacity = reading2capacity(acr);

	b->level = (acr * 100) / current_accum_capacity;

		/* if we're not really down, <3.3wV */
	if (b->batt_vol > bi->bat_pdata.low_voltage - 100) {

		/* No not indicate 0% */
		if (b->level < 1)
			b->level = 1;

		/* If we aren't really down, never go lower than 10 for this register (cannot reboot) */
		if (acr < DS2746_MIN_ACCUM_VALUE) {
				pr_info("ds2746: not low correction %d mV acr changed from %d to %d\n",
							 b->batt_vol, acr, DS2746_MIN_ACCUM_VALUE);
				acr = set_accum_value(DS2746_MIN_ACCUM_VALUE);
		}
	}

	if (b->level > 100)
		b->level = 100;
	
	/* Temperature */
	r_ain0 = (i2c_read(DS2746_AUX0_MSB) << 4) |
			 (i2c_read(DS2746_AUX0_LSB) >> 4);
			 
	b->temp_adc = r_ain0;
	b->batt_temp = ds2746_battery_read_temp(b);

	return charge_ended;
}

/**
 * Called by the work queue. Obtains a battery reading then
 * schedules the next read.
 */
static void ds2746_battery_work(struct work_struct *work)
{
	unsigned long next_update;
	int charge_ended;
	int am_i_supplied;
	if (bi->bat_pdata.block_charge) {
		bi->bat_pdata.block_charge(true);
		msleep(150);
	}

	charge_ended = ds2746_battery_read_status(bi);

	if (bi->bat_pdata.block_charge) {
		bi->bat_pdata.block_charge(false);
	}

	bi->charging_enabled = false;
	am_i_supplied = power_supply_am_i_supplied(bi->bat);
	if (am_i_supplied) {
		if (!wake_lock_active(&chrg_lock)){
			printk("%s: usb powered, aquiring wakelock", __func__);
			wake_lock(&chrg_lock);
		}
		next_update = msecs_to_jiffies(FAST_POLL);
		if (!charge_ended)
			bi->charging_enabled = true;
	}
	else {
		if (wake_lock_active(&chrg_lock)){
			printk("%s: usb not powered, releasing wakelock", __func__);
			wake_unlock(&chrg_lock);
		}
		next_update = msecs_to_jiffies(SLOW_POLL);
	}

	/* Check if supply change, reset history (to prevent from considering battery is charged) */
	if(am_i_supplied != bi->last_am_i_supplied)
		bi->batt_history_nb = 0;
	bi->last_am_i_supplied = am_i_supplied;

	power_supply_changed(bi->bat);
	queue_delayed_work(monitor_wqueue, &bat_work, next_update);
}

static int
ds2746_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	unsigned short acr;

	struct ds2746_platform_data pdata = {
		.resistance = DEFAULT_RSNS,
		.capacity = DEFAULT_BATTERY_RATING,
		.high_voltage = DEFAULT_HIGH_VOLTAGE,
		.low_voltage = DEFAULT_LOW_VOLTAGE,
	};

	// init the charger wakelock
	wake_lock_init(&chrg_lock, WAKE_LOCK_SUSPEND, "charger_wakelock");

	// Try and check if we actually have the ds2746 on this device
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("ds2746: DS-2746 chip not present");
		return -ENODEV;
	}
	bi = kzalloc(sizeof(*bi), GFP_KERNEL);
	if (!bi) {
		return -ENOMEM;
	}
	pr_info("ds2746: Initializing DS-2746 chip driver at addr: 0x%02x\n",
	       client->addr);
	pclient = client;

	if (client->dev.platform_data)
		pdata = *(struct ds2746_platform_data*)client->dev.platform_data;

	current_accum_capacity = pdata.capacity * pdata.resistance;
	current_accum_capacity /= DS2746_CURRENT_ACCUM_RES;

	bi->bat_pdata = pdata;
	bi->full_bat = 100;
	bi->last_s_value = 0;
	bi->batt_current = bi->batt_current_1 = bi->batt_current_2 = bi->batt_current_3 = 0;
	bi->batt_current_4 = bi->batt_current_5 = 0;
	bi->aver_batt_current = 0;
	bi->batt_history_nb = 0;
	bi->batt_temp = 250;
	bi->last_temp_adc = 0;
	bi->last_temp_index = 0;

	DBG("ds2746: resistance = %d, capacity = %d, "
		"high_voltage = %d, low_voltage = %d, softACR = %d\n",
		pdata.resistance, pdata.capacity,
		pdata.high_voltage, pdata.low_voltage,
		current_accum_capacity);

	/* Get accum value */
	acr = i2c_read(DS2746_CURRENT_ACCUM_LSB);
	acr |= i2c_read(DS2746_CURRENT_ACCUM_MSB) << 8;

	/* To allow boot, ignore too low values */
	if(acr < 2 * DS2746_MIN_ACCUM_VALUE)
		{
			pr_info("ds2746: acr is too low to boot (%d)\n", acr);
			acr = set_accum_value(2 * DS2746_MIN_ACCUM_VALUE);
		}

	/* Check registers mistake of bootloader */
	if(acr > DS2746_TOO_HIGH_ACCUM_VALUE)
		{
			pr_info("ds2746: acr is obviously too high (%d)\n", acr);
			pr_info("ds2746: maybe low value correction bootloader mistake, may be empty, set acr to 1\n");
			acr = set_accum_value(1);
		}

	/* Init a default value for offset_bias */
	i2c_write_signed(DS2746_ACCUM_BIAS, DS2746_ACCUM_BIAS_DEFAULT);

	INIT_DELAYED_WORK(&bat_work, ds2746_battery_work);

	// Create a worker queue just for us
	monitor_wqueue = create_singlethread_workqueue("ds2746-monitor");
	// Schedule a reading
	queue_delayed_work(monitor_wqueue, &bat_work, 1);

	ret = power_supply_register(&client->dev, &bat_ps);
	bi->bat = &bat_ps;

	return ret;
}

static int ds2746_detach_client(struct i2c_client *client)
{
//	i2c_detach_client(client);
	wake_lock_destroy(&chrg_lock);
	return 0;
}

static struct i2c_device_id ds2746_idtable[] = {
	{DS2746_NAME, 0},
};

#ifdef CONFIG_PM
static int ds2746_battery_suspend(struct i2c_client *client, pm_message_t pmesg)
{
	DBG("ds2746: Cancelling scheduled reads\n");
	cancel_delayed_work(&bat_work);
	flush_workqueue(monitor_wqueue);
	return 0;
}

static int ds2746_battery_resume(struct i2c_client *client)
{
	DBG("ds2746: Resuming scheduled reads\n");
	queue_delayed_work(monitor_wqueue, &bat_work, 1);
	return 0;
}

#else

#define ds2746_battery_suspend NULL
#define ds2746_battery_resume NULL

#endif

static struct i2c_driver ds2746_battery_driver = {
	.probe = ds2746_probe,
	.remove = ds2746_detach_client,
	.suspend = ds2746_battery_suspend,
	.resume = ds2746_battery_resume,
	.driver = {
		   .name = DS2746_NAME,
		   },
	.id_table = ds2746_idtable,
};

static int __init ds2746_battery_init(void)
{
	return i2c_add_driver(&ds2746_battery_driver);
}

static void __exit ds2746_battery_exit(void)
{
	i2c_del_driver(&ds2746_battery_driver);
}

module_init(ds2746_battery_init);
module_exit(ds2746_battery_exit);

MODULE_DESCRIPTION("Battery driver for Xperia X1");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Kern <pyrophobicman@gmail.com>, "
	      "Szabolcs Gyurko <szabolcs.gyurko@tlt.hu>, "
	      "Matt Reimer <mreimer@vpop.net>, "
	      "Anton Vorontsov <cbou@mail.ru>, "
	      "Carsten Wilhelm <carsten@wilhelm-net.de>");
