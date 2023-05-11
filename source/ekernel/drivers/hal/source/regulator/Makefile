ccflags-y += 	-I$(obj)/ \
		-I$(srctree)/include/melis/kernel/drivers \
		-I$(srctree)/ekernel/drivers/include \
		-I$(srctree)/ekernel/core/rt-thread/include
obj-$(CONFIG_AXP152_POWER) += axp152.o
obj-$(CONFIG_AXP2101_POWER) += axp2101.o
obj-y += axp_twi.o axp.o  core.o
obj-$(CONFIG_SOC_SUN8IW19P1) += sun8iw19p1/core.o
