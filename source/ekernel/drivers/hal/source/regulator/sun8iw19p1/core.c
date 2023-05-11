/*
 * Copyright (C) 2020 allwinnertech Ltd.
 */

#include <sunxi_hal_regulator.h>

extern struct regulator_ops axp_regulator_ops;

#define REGULATOR_TWI_PORT 4

int hal_regulator_get(unsigned int request_flag, struct regulator_dev *rdev)
{
	int rc;

	rdev->flag = request_flag;

	switch (REGULATOR_TYPE(rdev->flag)) {
	case AXP2101_REGULATOR: 
	case AXP152_REGULATOR:
		rdev->flag |= (REGULATOR_PMU_ADDR << AXP_ADDR_SHIFT) |
			      (REGULATOR_TWI_PORT << TWI_PORT_SHIFT);
		rdev->ops = &axp_regulator_ops;
		#ifdef CONFIG_AXP2101_POWER
		rdev->private = (void *)axp2101_regulators;
		#else
		rdev->private = (void *)axp152_regulators;
		#endif
		axp_twi_init(rdev);
		break;
	default:
		return -1;
	}
	return 0;
}
