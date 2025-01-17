/*
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp152_reg.h>
#include <private_boot0.h>

#define pmu_err(format, arg...) printf("[pmu]: " format, ##arg)
#define pmu_info(format, arg...) /*printf("[pmu]: "format,##arg)*/

#define VDD_SYS_VOL (920)
#define VOL_ON (1)

typedef struct _axp_contrl_info {
	char name[8];

	u16 min_vol;
	u16 max_vol;
	u16 cfg_reg_addr;
	u16 cfg_reg_mask;

	u16 step0_val;
	u16 split1_val;
	u16 step1_val;
	u16 ctrl_reg_addr;

	u16 ctrl_bit_ofs;
	u16 step2_val;
	u16 split2_val;
} axp_contrl_info;

static int pmu_set_vol(char *name, int set_vol, int onoff);

static axp_contrl_info axp_ctrl_tbl[] = {
	/*   name        min,     max,    reg,    mask,  step0,   split1_val,  step1,   ctrl_reg,           ctrl_bit */
	{ "dcdc1", 2000, 3500, AXP152_DC1OUT_VOL, 0x0f, 100, 0, 0,
	  AXP152_OUTPUT_CTL, 7 },
	{ "dcdc2", 700, 2275, AXP152_DC2OUT_VOL, 0x3f, 25, 0, 0,
	  AXP152_OUTPUT_CTL, 6 },
	{ "dcdc3", 700, 3500, AXP152_DC3OUT_VOL, 0x3f, 50, 0, 0,
	  AXP152_OUTPUT_CTL, 5 },
	{ "aldo1", 1200, 3300, AXP152_ALDO12OUT_VOL, 0xf0, 100, 0, 0,
	  AXP152_OUTPUT_CTL, 3 },
    /*{ "gpio2ldo", 1800, 3300, AXP152_GPIO2_LDO_MOD, 0x0f, 100, 0, 0,
	  AXP152_GPIO2_CTL, 1 },*/

};
#define PMU_POWER_KEY_STATUS AXP152_INTSTS1
#define PMU_POWER_KEY_OFFSET 0x2

int axp152_probe_power_key(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_INTSTS2, &reg_value)) {
		return -1;
	}
	reg_value &= (0x03);
	if (reg_value) {
		if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_INTSTS2,
				   reg_value | (0x01 << 1))) {
			return -1;
		}
	}
	return reg_value & 3;
}

static axp_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i    = 0;
	int size = ARRAY_SIZE(axp_ctrl_tbl);
	for (i = 0; i < size; i++) {
		if (!strncmp(name, axp_ctrl_tbl[i].name,
			     strlen(axp_ctrl_tbl[i].name))) {
			break;
		}
	}
	if (i >= size) {
		return NULL;
	}
	return (axp_ctrl_tbl + i);
}

static int pmu_set_vol(char *name, int set_vol, int onoff)
{
	u8 reg_value;
	axp_contrl_info *p_item = NULL;
	u8 base_step;
    u8 shift;
    
	if (set_vol <= 0)
		return 0;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	pmu_info(
		"name %s, min_vol %dmv, max_vol %d, cfg_reg 0x%x, cfg_mask 0x%x \
		step0_val %d, split1_val %d, step1_val %d, ctrl_reg_addr 0x%x, ctrl_bit_ofs %d\n",
		p_item->name, p_item->min_vol, p_item->max_vol,
		p_item->cfg_reg_addr, p_item->cfg_reg_mask, p_item->step0_val,
		p_item->split1_val, p_item->step1_val, p_item->ctrl_reg_addr,
		p_item->ctrl_bit_ofs);
        
    shift = (p_item->cfg_reg_addr == AXP152_ALDO12OUT_VOL) ? 4 : 0;
	if (set_vol < p_item->min_vol) {
		set_vol = p_item->min_vol;
	} else if (set_vol > p_item->max_vol) {
		set_vol = p_item->max_vol;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->cfg_reg_addr,
			  &reg_value)) {
        pmu_err("error read reg 0x%x!\n",p_item->cfg_reg_addr);
		return -1;
	}

	reg_value &= ~p_item->cfg_reg_mask;
	if (p_item->split2_val && (set_vol > p_item->split2_val)) {
		base_step = (p_item->split2_val - p_item->split1_val) /
			    p_item->step1_val;

		base_step += (p_item->split1_val - p_item->min_vol) /
			     p_item->step0_val;
		reg_value |=
			(base_step +
			 (set_vol - p_item->split2_val) / p_item->step2_val) << shift;
	} else if (p_item->split1_val && (set_vol > p_item->split1_val)) {
		if (p_item->split1_val < p_item->min_vol) {
			pmu_err("bad split val(%d) for %s\n",
				p_item->split1_val, name);
		}

		base_step = (p_item->split1_val - p_item->min_vol) /
			    p_item->step0_val;
		reg_value |=
			(base_step +
			 (set_vol - p_item->split1_val) / p_item->step1_val) << shift;
	} else { 
		reg_value |= ((set_vol - p_item->min_vol) / p_item->step0_val) << shift;
	}
    pmu_info("set reg 0x%x val to %d\n", p_item->cfg_reg_addr, reg_value);
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, p_item->cfg_reg_addr,
			   reg_value)) {
		pmu_err("unable to set %s\n", name);
		return -1;
	}
    
    /*      set on/off       */
    if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		pmu_err("unable to onoff %s\n", name);
		return -1;
	}
	return 0;
}

int axp152_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc3", set_vol, 1);
}

int axp152_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("dcdc1", set_vol, 1);
}

int axp152_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("aldo1", set_vol, 1);
}

int axp152_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc2", set_vol, onoff);
}

int axp152_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP152_DEVICE_ADDR, AXP152_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_VERSION, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0x0F;
	if (pmu_type == AXP152_CHIP_ID) {
		/* pmu type AXP152 */
		printf("PMU: AXP152\n");
		return AXP152_CHIP_ID;
	}
	printf("unknown PMU\n");
	return -1;
}
