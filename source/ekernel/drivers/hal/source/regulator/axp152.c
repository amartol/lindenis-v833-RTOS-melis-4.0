/*
 * Copyright (C) 2020 allwinnertech Ltd.
 */

#include <sunxi_hal_regulator.h>
#include <type.h>

#define AXP152_PWR_OP_MODE		0x01
#define AXP152_LDO3456_DC1234_CTRL	0x12
#define AXP152_ALDO_OP_MODE		0x13
#define AXP152_LDO0_CTRL		0x15
#define AXP152_DCDC2_V_OUT		0x23
#define AXP152_DCDC2_V_SCAL		0x25
#define AXP152_DCDC1_V_OUT		0x26
#define AXP152_DCDC3_V_OUT		0x27
#define AXP152_ALDO12_V_OUT		0x28
#define AXP152_DLDO1_V_OUT		0x29
#define AXP152_DLDO2_V_OUT		0x2a
#define AXP152_DCDC4_V_OUT		0x2b
#define AXP152_V_OFF			0x31
#define AXP152_OFF_CTRL			0x32
#define AXP152_PEK_KEY			0x36
#define AXP152_DCDC_FREQ		0x37
#define AXP152_DCDC_MODE		0x80


static const struct regulator_linear_range axp152_dcdc2_ranges[] = {
	REGULATOR_LINEAR_RANGE(700000, 0x0, 0x3F, 25000),
};

static const struct regulator_linear_range axp152_dcdc3_ranges[] = {
	REGULATOR_LINEAR_RANGE(700000, 0x0, 0x3F, 50000),
};

static const struct regulator_linear_range axp152_dcdc4_ranges[] = {
	REGULATOR_LINEAR_RANGE(700000, 0x0, 0x7F, 25000),
};

const struct regulator_desc axp152_regulators[] = {
	AXP_DESC(2000, 3500, 100, AXP152_DCDC1_V_OUT, 0x0f,
		 AXP152_LDO3456_DC1234_CTRL, BIT(7)),
	AXP_DESC_RANGES(axp152_dcdc2_ranges, AXP152_DCDC2_V_OUT, 0x3f,
			AXP152_LDO3456_DC1234_CTRL, BIT(6)),
	AXP_DESC_RANGES(axp152_dcdc3_ranges, AXP152_DCDC3_V_OUT, 0x3f,
			AXP152_LDO3456_DC1234_CTRL, BIT(5)),
	AXP_DESC_RANGES(axp152_dcdc4_ranges, AXP152_DCDC4_V_OUT, 0x7f,
			AXP152_LDO3456_DC1234_CTRL, BIT(4)),
	AXP_DESC(1700, 3300, 100, AXP152_ALDO12_V_OUT, 0x0f, AXP152_LDO3456_DC1234_CTRL,
		 BIT(2)), // aldo2  
	AXP_DESC(1200, 3300, 100, AXP152_ALDO12_V_OUT, 0xf0, AXP152_LDO3456_DC1234_CTRL,
		 BIT(3)), // aldo1        
    AXP_DESC(700, 3300, 100, AXP152_DLDO1_V_OUT, 0x1f, AXP152_LDO3456_DC1234_CTRL,
		 BIT(1)),
	AXP_DESC(700, 3300, 100, AXP152_DLDO2_V_OUT, 0x1f, AXP152_LDO3456_DC1234_CTRL,
		 BIT(0)),
};
