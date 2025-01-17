/*
 * cameras power on and off drivers.
 *
 * Copyright (c) 2019 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zheng ZeQun <zequnzheng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <videodev2.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <v4l2-device.h>
#include <media/v4l2-mediabus.h>

#include "sensor_power.h"

#define MCLK0              (24*1000*1000)
#define MCLK1              (24*1000*1000)

#define SENSOR_POWER_NAME "vin_sensor_power"

uint iovdd0_vol = 0;
uint avdd0_vol = 0;
uint dvdd0_vol = 0;
uint iovdd1_vol = 0;
uint avdd1_vol = 0;
uint dvdd1_vol = 0;
uint mclk0 = 0;
uint mclk1 = 0;

module_param(iovdd0_vol, uint, S_IRUGO | S_IWUSR);
module_param(avdd0_vol, uint, S_IRUGO | S_IWUSR);
module_param(dvdd0_vol, uint, S_IRUGO | S_IWUSR);
module_param(iovdd1_vol, uint, S_IRUGO | S_IWUSR);
module_param(avdd1_vol, uint, S_IRUGO | S_IWUSR);
module_param(dvdd1_vol, uint, S_IRUGO | S_IWUSR);
module_param(mclk0, uint, S_IRUGO | S_IWUSR);
module_param(mclk1, uint, S_IRUGO | S_IWUSR);

/* ------------------------ get dts info ------------------------------- */
static int sensor_get_value_int(struct device_node *np, const char *name,
			  u32 *output)
{
	int ret;

	ret = of_property_read_u32(np, name, output);
	if (ret) {
		*output = 0;
		sensor_power_err("fetch %s from device_tree failed\n", name);
		return -EINVAL;
	}
	return 0;
}

static int sensor_get_gpio_info(struct device_node *np, const char *name,
			 struct gpio_config *gc)
{
	int gnum;

	gnum = of_get_named_gpio_flags(np, name, 0, (enum of_gpio_flags *)gc);
	if (!gpio_is_valid(gnum)) {
		gc->gpio = GPIO_INDEX_INVALID;
		sensor_power_dbg("fetch %s from device_tree failed\n", name);
		return -EINVAL;
	}

	return 0;
}

static int sensor_os_gpio_set(struct gpio_config *gc)
{
	char pin_name[32];
	__u32 config;

	if (gc == NULL)
		return -1;
	if (gc->gpio == GPIO_INDEX_INVALID)
		return -1;

	if (!IS_AXP_PIN(gc->gpio)) {
		/* valid pin of sunxi-pinctrl,
		 * config pin attributes individually.
		 */
		sunxi_gpio_to_name(gc->gpio, pin_name);
		config =
		    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, gc->mul_sel);
		pin_config_set(SUNXI_PINCTRL, pin_name, config);
		if (gc->pull != GPIO_PULL_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, gc->pull);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (gc->drv_level != GPIO_DRVLVL_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,
					      gc->drv_level);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (gc->data != GPIO_DATA_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, gc->data);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
	} else {
		/* valid pin of axp-pinctrl,
		 * config pin attributes individually.
		 */
		sunxi_gpio_to_name(gc->gpio, pin_name);
		config =
		    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, gc->mul_sel);
		pin_config_set(AXP_PINCTRL, pin_name, config);
		if (gc->data != GPIO_DATA_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, gc->data);
			pin_config_set(AXP_PINCTRL, pin_name, config);
		}
	}
	return 0;
}

/*
 *set the gpio io status
 */
static int sensor_gpio_set_status(struct sensor_power_dev *sensor_power, enum sensor_gpio_type gpio_id, unsigned int status)
{
	struct gpio_config gc_def;
	struct gpio_config *gc = NULL;

	gc = &sensor_power->gpio[gpio_id];
	if (gc->gpio == GPIO_INDEX_INVALID)
		return -1;

	memcpy(&gc_def, &gc, sizeof(struct gpio_config));

	if (status == 0)
		gc_def.mul_sel = GPIO_DISABLE;

	if (status == 3)
		gc_def.mul_sel = 3;

	if (sensor_os_gpio_set(&gc_def) < 0)
		return -1;

	if (status == 3)
		return 0;

	if (status == 1)
		gpio_direction_output(gc->gpio, 0);
	else
		gpio_direction_input(gc->gpio);

	return 0;
}

static int sensor_os_gpio_write(u32 gpio, __u32 out_value)
{
	if (gpio == GPIO_INDEX_INVALID)
		return 0;

	gpio_direction_output(gpio, 0);
	__gpio_set_value(gpio, out_value);

	return 0;
}

static int sensor_gpio_write(struct sensor_power_dev *sensor_power, enum sensor_gpio_type gpio_id, unsigned int out_value)
{
	struct gpio_config *gc = NULL;

	gc = &sensor_power->gpio[gpio_id];

	return sensor_os_gpio_write(gc->gpio, out_value);
}

/*
 *enable/disable pmic channel
 */
static int sensor_set_pmu_channel(struct sensor_power_dev *sensor_power,
			enum sensor_pmic_channel pmic_ch, enum sensor_on_off on_off)
{
	int ret = 0;
	static int def_vol[MAX_POW_NUM] = {1800000, 2800000, 1200000,
					1800000, 1800000, 1800000};
	struct sensor_power_cfg *power;

	power  = &sensor_power->power[0];
	if (on_off == OFF) {
		if (!power[pmic_ch].pmic)
			return 0;
		ret = regulator_disable(power[pmic_ch].pmic);
		regulator_put(power[pmic_ch].pmic);
		power[pmic_ch].pmic = NULL;
	} else {
		if (!power[pmic_ch].pmic || !power[pmic_ch].power_vol)
			return 0;
		ret = regulator_set_voltage(power[pmic_ch].pmic,
					  power[pmic_ch].power_vol,
					  def_vol[pmic_ch]);
		sensor_power_dbg("set regulator %s = %d,return %x\n",
			pmic_ch == 0 ? "IOVDD" : (pmic_ch == 1) ? "AVDD" : "DVDD",
			power[pmic_ch].power_vol, ret);
		ret = regulator_enable(power[pmic_ch].pmic);
	}

	return ret;
}

/*
 *enable/disable master clock
 */
static int sensor_set_mclk(struct sensor_power_dev *sensor_power, enum sensor_on_off on_off)
{
	struct sensor_mclk_info *mclk = NULL;
	struct sensor_power_cfg *power = NULL;
	unsigned int mclk_mode;
	char pin_name[20] = "";
	int mclk_id = 0;
	int ret;

	if (sensor_power == NULL)
		return -1;
	if (!sensor_power->mclk.mclk || !sensor_power->mclk.clk_24m || !sensor_power->mclk.clk_pll)
		return -1;

	if (sensor_power->mclk.mclk_id == -1)
		mclk_id = sensor_power->id;
	else
		mclk_id = sensor_power->mclk.mclk_id;
	if (mclk_id < 0) {
		sensor_power_err("get mclk id failed\n");
		return -1;
	}

	mclk = &sensor_power->mclk;

	switch (on_off) {
	case ON:
		sprintf(pin_name, "mclk%d-default", mclk_id);
		break;
	case OFF:
		sprintf(pin_name, "mclk%d-sleep", mclk_id);
		break;
	default:
		return -1;
	}

	if (on_off && mclk->use_count++ > 0)
		return 0;
	else if (!on_off && (mclk->use_count == 0 || --mclk->use_count > 0))
		return 0;

	switch (on_off) {
	case ON:
		sensor_power_dbg("sensor mclk on, use_count %d!\n", mclk->use_count);
		if (mclk->mclk) {
			if (clk_prepare_enable(mclk->mclk)) {
				sensor_power_err("csi master clock enable error\n");
				return -1;
			}
		} else {
			sensor_power_err("csi master%d clock is null\n", mclk_id);
			return -1;
		}
		break;
	case OFF:
		sensor_power_dbg("sensor mclk off, use_count %d!\n", mclk->use_count);
		if (mclk->mclk) {
			clk_disable_unprepare(mclk->mclk);
		} else {
			sensor_power_err("csi master%d clock is null\n", mclk_id);
			return -1;
		}
		break;
	default:
		return -1;
	}

	mclk->pin = devm_pinctrl_get_select(&sensor_power->pdev->dev, pin_name);
	if (IS_ERR_OR_NULL(mclk->pin)) {
		sensor_power_err("mclk%d request pin handle failed!\n", mclk_id);
		return -EINVAL;
	}

	if (on_off) {
		power  = &sensor_power->power[0];
		if (power[IOVDD].power_vol && (power[IOVDD].power_vol <= 1800000))
			mclk_mode = MCLK_POWER_1800;
		else
			mclk_mode = MCLK_POWER_3300;
		sensor_power_dbg("IOVDD power vol is %d, mclk mode is %d\n", power[IOVDD].power_vol, mclk_mode);

		ret = sunxi_sel_pio_mode(mclk->pin, mclk_mode);
		if (ret) {
			sensor_power_err("Cannot select mclk pio power mode!\n");
			return ret;
		}
	}
	return 0;
}

/*
 *set frequency of master clock
 */
static int vin_set_mclk_freq(struct sensor_power_dev *sensor_power, unsigned long freq)
{
	struct clk *mclk_src = NULL;
	int mclk_id = 0;

	if (sensor_power == NULL)
		return -1;
	if (!sensor_power->mclk.mclk || !sensor_power->mclk.clk_24m || !sensor_power->mclk.clk_pll)
		return -1;

	if (sensor_power->mclk.mclk_id == -1)
		mclk_id = sensor_power->id;
	else
		mclk_id = sensor_power->mclk.mclk_id;
	if (mclk_id < 0) {
		sensor_power_err("get mclk id failed\n");
		return -1;
	}

	if (freq == 24000000 || freq == 12000000 || freq == 6000000) {
		if (sensor_power->mclk.clk_24m) {
			mclk_src = sensor_power->mclk.clk_24m;
		} else {
			sensor_power_err("csi master clock 24M source is null\n");
			return -1;
		}
	} else {
		if (sensor_power->mclk.clk_pll) {
			mclk_src = sensor_power->mclk.clk_pll;
		} else {
			sensor_power_err("csi master clock pll source is null\n");
			return -1;
		}
	}

	if (sensor_power->mclk.mclk) {
		if (clk_set_parent(sensor_power->mclk.mclk, mclk_src)) {
			sensor_power_err("set mclk%d source failed!\n", mclk_id);
			return -1;
		}
		if (clk_set_rate(sensor_power->mclk.mclk, freq)) {
			sensor_power_err("set csi master%d clock error\n", mclk_id);
			return -1;
		}
		sensor_power_dbg("mclk%d set rate %ld, get rate %ld\n", mclk_id,
			freq, clk_get_rate(sensor_power->mclk.mclk));
	} else {
		sensor_power_err("csi master clock is null\n");
		return -1;
	}
	return 0;
}

static int sensor_power_on(struct sensor_power_dev *sensor_power)
{
	if (sensor_power->id == 0 || sensor_power->id == 1) { /* sensor0 or sensor1 power on */
		sensor_set_pmu_channel(sensor_power, IOVDD, ON);
		sensor_set_pmu_channel(sensor_power, DVDD, ON);
		sensor_set_pmu_channel(sensor_power, AVDD, ON);
		usleep_range(1000, 1200);

		sensor_gpio_set_status(sensor_power, RESET, 1);
		sensor_gpio_set_status(sensor_power, PWDN, 1);
		sensor_gpio_write(sensor_power, RESET, SENSOR_GPIO_HIGH);
		sensor_gpio_write(sensor_power, PWDN, SENSOR_GPIO_HIGH);
		usleep_range(1000, 1200);

		sensor_set_mclk(sensor_power, ON);
		vin_set_mclk_freq(sensor_power, sensor_power->mclk.frequency);
		usleep_range(1000, 1200);
	}
	return 0;
};

static void sensor_power_off(struct sensor_power_dev *sensor_power)
{
	if (sensor_power->id == 0 || sensor_power->id == 1) { /* sensor0 or sensor1 power off */
		sensor_set_pmu_channel(sensor_power, IOVDD, OFF);
		sensor_set_pmu_channel(sensor_power, DVDD, OFF);
		sensor_set_pmu_channel(sensor_power, AVDD, OFF);
		sensor_gpio_set_status(sensor_power, RESET, 1);
		sensor_gpio_set_status(sensor_power, PWDN, 1);
		sensor_gpio_write(sensor_power, RESET, SENSOR_GPIO_LOW);
		sensor_gpio_write(sensor_power, PWDN, SENSOR_GPIO_LOW);
		sensor_set_mclk(sensor_power, OFF);
		usleep_range(1000, 1200);
	}
}
/* ----------------------------------------------------------------------- */

static int sensor_power_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct sensor_power_dev *sensor_power = NULL;
	char property[32] = {0,};
	int ret = 0;

	sensor_power_dbg(" *************** %s ***************\n", __func__);
	if (np == NULL) {
		sensor_power_err("sensor_helper failed to get of node\n");
		return -ENODEV;
	}

	sensor_power = kzalloc(sizeof(struct sensor_power_dev), GFP_KERNEL);
	if (!sensor_power) {
		ret = -ENOMEM;
		goto ekzalloc;
	}

	/************************** get dts ************************************/
	of_property_read_u32(np, "device_id", &pdev->id);
	if (pdev->id < 0) {
		sensor_power_err("sensor_helper failed to get device id\n");
		ret = -EINVAL;
		goto freedev;
	}
	sensor_power->id = pdev->id;
	sensor_power->pdev = pdev;

	if (sensor_power->id == 0) {
		sensor_power->power[IOVDD].power_vol = iovdd0_vol;
		sensor_power->power[AVDD].power_vol = avdd0_vol;
		sensor_power->power[DVDD].power_vol = dvdd0_vol;
		sensor_power->mclk.frequency = mclk0;
		if (!sensor_power->mclk.frequency)
			sensor_power->mclk.frequency = MCLK0;
		else
			roundup(sensor_power->mclk.frequency, 1000000);
	} else if (sensor_power->id == 1) {
		sensor_power->power[IOVDD].power_vol = iovdd1_vol;
		sensor_power->power[AVDD].power_vol = avdd1_vol;
		sensor_power->power[DVDD].power_vol = dvdd1_vol;
		sensor_power->mclk.frequency = mclk1;
		if (!sensor_power->mclk.frequency)
			sensor_power->mclk.frequency = MCLK1;
		else
			roundup(sensor_power->mclk.frequency, 1000000);
	}

	/* IOVDD */
	snprintf(property, sizeof(property), "sensor%d_iovdd", pdev->id);
	sensor_power->power[IOVDD].pmic = regulator_get_optional(dev, property);
	if (IS_ERR(sensor_power->power[IOVDD].pmic)) {
		sensor_power_warn("%s: cannot get %s supply, setting it to NULL!\n", __func__, property);
		sensor_power->power[IOVDD].pmic = NULL;
	}
	if (!sensor_power->power[IOVDD].power_vol) {
		snprintf(property, sizeof(property), "sensor%d_iovdd_vol", pdev->id);
		sensor_get_value_int(np, property, &sensor_power->power[IOVDD].power_vol);
	} else
		roundup(sensor_power->power[IOVDD].power_vol, 100000);

	/* AVDD */
	snprintf(property, sizeof(property), "sensor%d_avdd", pdev->id);
	sensor_power->power[AVDD].pmic = regulator_get_optional(dev, property);
	if (IS_ERR(sensor_power->power[AVDD].pmic)) {
		sensor_power_warn("%s: cannot get %s supply, setting it to NULL!\n", __func__, property);
		sensor_power->power[AVDD].pmic = NULL;
	}
	if (!sensor_power->power[AVDD].power_vol) {
		snprintf(property, sizeof(property), "sensor%d_avdd_vol", pdev->id);
		sensor_get_value_int(np, property, &sensor_power->power[AVDD].power_vol);
	} else
		roundup(sensor_power->power[AVDD].power_vol, 100000);

	/* DVDD */
	snprintf(property, sizeof(property), "sensor%d_dvdd", pdev->id);
	sensor_power->power[DVDD].pmic = regulator_get_optional(dev, property);
	if (IS_ERR(sensor_power->power[DVDD].pmic)) {
		sensor_power_warn("%s: cannot get %s supply, setting it to NULL!\n", __func__, property);
		sensor_power->power[DVDD].pmic = NULL;
	}
	if (!sensor_power->power[DVDD].power_vol) {
		snprintf(property, sizeof(property), "sensor%d_dvdd_vol", pdev->id);
		sensor_get_value_int(np, property, &sensor_power->power[DVDD].power_vol);
	} else
		roundup(sensor_power->power[DVDD].power_vol, 100000);

	/* reset */
	snprintf(property, sizeof(property), "sensor%d_reset", pdev->id);
	ret = sensor_get_gpio_info(np, property, &sensor_power->gpio[RESET]);
	if (ret < 0)
		sensor_power_err("get %s error\n", property);
	if (sensor_power->gpio[RESET].gpio != GPIO_INDEX_INVALID) {
		ret = gpio_request(sensor_power->gpio[RESET].gpio, NULL);
		if (ret < 0)
			sensor_power_err("RESET: gpio%d request failed!\n", sensor_power->gpio[RESET].gpio);
	}

	/* pwdn */
	snprintf(property, sizeof(property), "sensor%d_pwdn", pdev->id);
	ret = sensor_get_gpio_info(np, property, &sensor_power->gpio[PWDN]);
	if (ret < 0)
		sensor_power_err("get %s error\n", property);
	if (sensor_power->gpio[PWDN].gpio != GPIO_INDEX_INVALID) {
		ret = gpio_request(sensor_power->gpio[PWDN].gpio, NULL);
		if (ret < 0)
			sensor_power_err("PWDN: gpio%d request failed!\n", sensor_power->gpio[PWDN].gpio);
	}

	/* MCLK */
	snprintf(property, sizeof(property), "sensor%d_mclk_id", pdev->id);
	sensor_get_value_int(np, property, &sensor_power->mclk.mclk_id);

	sensor_power->mclk.mclk = of_clk_get(np, 0);
	if (IS_ERR_OR_NULL(sensor_power->mclk.mclk)) {
		sensor_power_err("Get mclk%d failed!\n", sensor_power->mclk.mclk_id);
		sensor_power->mclk.mclk = NULL;
	}
	sensor_power->mclk.clk_24m = of_clk_get(np, 1);
	if (IS_ERR_OR_NULL(sensor_power->mclk.clk_24m)) {
		sensor_power_err("Get mclk%d_24m failed!\n", sensor_power->mclk.mclk_id);
		sensor_power->mclk.clk_24m = NULL;
	}
	sensor_power->mclk.clk_pll = of_clk_get(np, 2);
	if (IS_ERR_OR_NULL(sensor_power->mclk.clk_pll)) {
		sensor_power_err("Get mclk%d_pll failed!\n", sensor_power->mclk.mclk_id);
		sensor_power->mclk.clk_pll = NULL;
	}

	sensor_power_dbg("IOVDD : %d\n", sensor_power->power[IOVDD].power_vol);
	sensor_power_dbg("AVDD : %d\n", sensor_power->power[AVDD].power_vol);
	sensor_power_dbg("DVDD : %d\n", sensor_power->power[DVDD].power_vol);
	sensor_power_dbg("reset : pin=%d  mul-sel=%d  drive=%d  pull=%d  data=%d\n",
		sensor_power->gpio[RESET].gpio, sensor_power->gpio[RESET].mul_sel, sensor_power->gpio[RESET].drv_level,
		sensor_power->gpio[RESET].pull, sensor_power->gpio[RESET].data);
	sensor_power_dbg("pwdn : pin=%d  mul-sel=%d  drive=%d  pull=%d  data=%d\n",
		sensor_power->gpio[PWDN].gpio, sensor_power->gpio[PWDN].mul_sel, sensor_power->gpio[PWDN].drv_level,
		sensor_power->gpio[PWDN].pull, sensor_power->gpio[PWDN].data);
	sensor_power_dbg("mlck: mclk id = %d, mclk frequency is %ld\n", sensor_power->mclk.mclk_id, sensor_power->mclk.frequency);
	sensor_power_dbg(" *************************************************\n");

	sensor_power_on(sensor_power);
	platform_set_drvdata(pdev, sensor_power);
	return 0;
freedev:
	kfree(sensor_power);
ekzalloc:
	sensor_power_err("sensor_power_probe probe err!\n");
	return ret;
}

static int sensor_power_remove(struct platform_device *pdev)
{
	struct sensor_power_dev *sensor_power = platform_get_drvdata(pdev);

	sensor_power_off(sensor_power);
	if (sensor_power->power[IOVDD].pmic)
		regulator_put(sensor_power->power[IOVDD].pmic);
	if (sensor_power->power[AVDD].pmic)
		regulator_put(sensor_power->power[AVDD].pmic);
	if (sensor_power->power[DVDD].pmic)
		regulator_put(sensor_power->power[DVDD].pmic);
	if (sensor_power->gpio[RESET].gpio != GPIO_INDEX_INVALID)
		gpio_free(sensor_power->gpio[RESET].gpio);
	if (sensor_power->gpio[PWDN].gpio != GPIO_INDEX_INVALID)
		gpio_free(sensor_power->gpio[PWDN].gpio);
	kfree(sensor_power);
	return 0;
}

static const struct of_device_id sunxi_sensor_power_match[] = {
	{.compatible = "allwinner,sunxi-sensor",},
	{},
};

static struct platform_driver sensor_power_driver = {
	.probe = sensor_power_probe,
	.remove = sensor_power_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_POWER_NAME,
		.of_match_table = sunxi_sensor_power_match,
	},
};

static __init int sensor_power_init(void)
{
	return platform_driver_register(&sensor_power_driver);
}

static __exit void sensor_power_exit(void)
{
	platform_driver_unregister(&sensor_power_driver);
}

module_init(sensor_power_init);
module_exit(sensor_power_exit);

MODULE_AUTHOR("zhengzequn");
MODULE_DESCRIPTION("sensor power on and off");
MODULE_LICENSE("GPL");
