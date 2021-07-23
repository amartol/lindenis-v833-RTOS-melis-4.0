/*
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <arch/clock.h>
#include <arch/uart.h>

static void set_circuits_analog(void)
{
	/*calibration circuits analog enable*/
	/*sunxi_clear_bit(RES_CAL_CTRL_REG, BIT(1));*/
	/*sunxi_clear_bit(RES_CAL_CTRL_REG, BIT(0)); */
	setbits_le32(VDD_SYS_PWROFF_GATING_REG, 1 << VDD_ADDA_OFF_GATING);

	clrsetbits_le32(RES_CAL_CTRL_REG, 0 << 0, 1 << 0);
}

void set_factor_n(int factor_n, int reg)
{
	int reg_val;
	reg_val = readl(reg);
	reg_val &= ~(0xff << 8);
	reg_val |= (factor_n << 8);
	writel(reg_val, reg);
}

void set_pll_cpux_axi(void)
{
	__u32 reg_val;

	/*select CPUX  clock src: OSC24M, AXI divide ratio is 3, system apb clk ratio is 4*/
	writel((0<<24) | (3<<8) | (2<<0), CCMU_CPUX_AXI_CFG_REG);
	udelay(1);

	/* set default val: clk is 408M  , PLL_OUTPUT= 24M*N/(M*P)*/
	writel((0x8a002300), CCMU_PLL_CPUX_CTRL_REG);

	/* lock enable */
	reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	reg_val |= (1<<29);
	writel(reg_val, CCMU_PLL_CPUX_CTRL_REG);

	/* enable pll */
	reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	reg_val |=  (1<<31);
	writel(reg_val, CCMU_PLL_CPUX_CTRL_REG);

	/* wait PLL_CPUX stable */
#ifndef FPGA_PLATFORM
	while (!(readl(CCMU_PLL_CPUX_CTRL_REG) & (0x1 << 28)))
		;
	udelay(1);
#endif
	/* lock disable */
	reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	reg_val &= ~(1<<29);
	writel(reg_val, CCMU_PLL_CPUX_CTRL_REG);

	/* set and change cpu clk src to PLL_CPUX,  PLL_CPUX:AXI0 = 408M:136M */
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &=  ~(0x03 << 24);
	reg_val |=  (0x03 << 24);
	writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
	udelay(1);
}


static void pll_enable(u32 pll_addr)
{
	u32 reg_value = 0;
	reg_value = readl(pll_addr);
	reg_value |= ((0x1U<<31)|(0x1<<29));
	writel(reg_value, pll_addr);
}

#ifndef FPGA_PLATFORM
static void pll_disable(u32 pll_addr)
{
	u32 reg_value = 0;
	reg_value = readl(pll_addr);
	reg_value &= (~((0x1U<<31)|(0x1<<29)));
	writel(reg_value, pll_addr);
}

static int  wait_pll_stable(u32 pll_addr, u32 lock_bit)
{
	u32 reg_value = 0;
	u32 counter = 0;

	while (1) {
		pll_enable(pll_addr);
		counter = 0;
		do {
			reg_value = readl(pll_addr);
			reg_value = reg_value & (0x1 << lock_bit);
			if (reg_value) {
				return 0;
			}
			counter++;
		} while (counter < 0x7ff);

		pll_disable(pll_addr);
	}

	return -1;
}
#endif

void set_pll_periph0(void)
{
	__u32 reg_val;

	/*change  psi/ahb src to OSC24M before set pll6 */
	reg_val = readl(CCMU_PSI_AHB1_AHB2_CFG_REG);
	reg_val &= (~(0x3<<24));
	writel(reg_val, CCMU_PSI_AHB1_AHB2_CFG_REG);

	/* disabe PLL: 600M(1X)  1200M(2x) */
	reg_val = readl(CCMU_PLL_PERI0_CTRL_REG);
	reg_val &= ~(1<<31);
	writel(reg_val, CCMU_PLL_PERI0_CTRL_REG);

	/* set factor n=80 , 1920Mhz*/
	reg_val = readl(CCMU_PLL_PERI0_CTRL_REG);
	reg_val &= (~((0xFF<<8)|(0x1<<1)|(0x1<<0)));
	reg_val |= ((0x4F<<8)|(0x0<<1)|(0x0<<0));
	writel(reg_val, CCMU_PLL_PERI0_CTRL_REG);
	udelay(10);

	/*enable PLL_PERIPH0*/
	pll_enable(CCMU_PLL_PERI0_CTRL_REG);
#ifndef FPGA_PLATFORM
	wait_pll_stable(CCMU_PLL_PERI0_CTRL_REG, 28);
	udelay(10);
#endif

	/* lock disable */
	reg_val = readl(CCMU_PLL_PERI0_CTRL_REG);
	reg_val &= (~(1<<29));
	writel(reg_val, CCMU_PLL_PERI0_CTRL_REG);

	/* set default factor n=0x31, 1200M */
	reg_val = readl(CCMU_PLL_PERI0_CTRL_REG);
	reg_val &= ~((0xFF<<8) | (0x3 << 0));
	reg_val |= (0x31<<8);
	writel(reg_val, CCMU_PLL_PERI0_CTRL_REG);

	/* lock enable */
	reg_val = readl(CCMU_PLL_PERI0_CTRL_REG);
	reg_val |= (1<<29);
	writel(reg_val, CCMU_PLL_PERI0_CTRL_REG);
	udelay(10);

#ifndef FPGA_PLATFORM
	while (!(readl(CCMU_PLL_PERI0_CTRL_REG) & (0x1 << 28)))
		;
	udelay(20);
#endif

}


void set_ahb(void)
{
	/* PLL6:AHB1:APB1 = 600M:200M:100M */
	writel((2<<0) | (0<<8), CCMU_PSI_AHB1_AHB2_CFG_REG);
	writel((0x03 << 24)|readl(CCMU_PSI_AHB1_AHB2_CFG_REG), CCMU_PSI_AHB1_AHB2_CFG_REG);
	udelay(1);
	/*PLL6:AHB3 = 600M:200M*/
	writel((2<<0) | (0<<8), CCMU_AHB3_CFG_GREG);
	writel((0x03 << 24)|readl(CCMU_AHB3_CFG_GREG), CCMU_AHB3_CFG_GREG);
}

void set_apb(void)
{
	/*PLL6:APB1 = 600M:100M */
	writel((2<<0) | (1<<8), CCMU_APB1_CFG_GREG);
	writel((0x03 << 24)|readl(CCMU_APB1_CFG_GREG), CCMU_APB1_CFG_GREG);
	udelay(1);
}

/* SRAMC's clk source is AHB1, so the AHB1 clk must less than 100M */
void set_pll_ahb_for_secure(void)
{
	/* PLL6:AHB1 = 600M:100M */
	/* div M=3 N=2*/
	writel((2<<0) | (1<<8), CCMU_PSI_AHB1_AHB2_CFG_REG);
	writel((0x03 << 24)|readl(CCMU_PSI_AHB1_AHB2_CFG_REG), CCMU_PSI_AHB1_AHB2_CFG_REG);
	udelay(1);
}


void set_pll_dma(void)
{
	u32 tmp;

	/*dma reset*/
	writel(readl(CCMU_DMA_BGR_REG)  | (1 << 16), CCMU_DMA_BGR_REG);
	/*gating clock for dma pass*/
	writel(readl(CCMU_DMA_BGR_REG) | (1 << 0), CCMU_DMA_BGR_REG);

	tmp = readl(CCMU_MBUS_MST_CLK_GATING_REG) | (1<<0);
	writel(tmp, CCMU_MBUS_MST_CLK_GATING_REG);
}

void set_pll_mbus(void)
{
	__u32 reg_val;

	/*reset mbus domain*/
	reg_val = 1<<30;
	writel(reg_val, CCMU_MBUS_CFG_REG);
	udelay(1);
	/*mbus clk = dram_clk/4*/
}


static void set_cpu_step(void)
{
	u32 reg_val;

	reg_val = readl(SUNXI_CCM_BASE + 0x400);
	reg_val &= (~(0x7 << 28));
	reg_val |= (0x01 << 28);
	writel(reg_val, SUNXI_CCM_BASE + 0x400);
}

void sunxi_board_pll_init(void)
{
	printf("set pll start\n");
	set_circuits_analog();
	set_cpu_step();
	set_pll_cpux_axi();
	set_pll_periph0();
	set_ahb();
	set_apb();
	set_pll_dma();
	set_pll_mbus();

	printf("set pll end\n");
	return ;
}


void sunxi_board_clock_reset(void)
{
	__u32 reg_val;
	/* set ahb, apb to default, use OSC24M */
	reg_val = readl(CCMU_PSI_AHB1_AHB2_CFG_REG);
	reg_val &= (~(0x3<<24));
	writel(reg_val, CCMU_PSI_AHB1_AHB2_CFG_REG);

	reg_val = readl(CCMU_APB1_CFG_GREG);
	reg_val &= (~(0x3<<24));
	writel(reg_val, CCMU_APB1_CFG_GREG);

	/* set cpux pll to default, use OSC24M */
	writel(0x0301, CCMU_CPUX_AXI_CFG_REG);
	return ;
}


int sunxi_key_clock_open(void)
{
	uint reg_val = 0, i = 0;

	/* reset */
	reg_val = readl(CCMU_GPADC_BGR_REG);
	reg_val &= ~(1<<16);
	writel(reg_val, CCMU_GPADC_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg_val |=  (1<<16);
	writel(reg_val, CCMU_GPADC_BGR_REG);

	/* enable KEYADC gating */
	reg_val = readl(CCMU_GPADC_BGR_REG);
	reg_val |= (1<<0);
	writel(reg_val, CCMU_GPADC_BGR_REG);

	return 0;
}

int sunxi_key_clock_close(void)
{
	uint reg_val = 0;

	/* disable KEYADC gating */
	reg_val = readl(CCMU_GPADC_BGR_REG);
	reg_val &= ~(1<<0);
	writel(reg_val, CCMU_GPADC_BGR_REG);

	return 0;
}

void sunxi_clock_init_uart(int port)
{
	u32 i, reg;
	/* reset */
	reg = readl(CCMU_UART_BGR_REG);
	reg &= ~(1<<(CCM_UART_RST_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg |= (1 << (CCM_UART_RST_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	/* gate */
	reg = readl(CCMU_UART_BGR_REG);
	reg &= ~(1<<(CCM_UART_GATING_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg |= (1 << (CCM_UART_GATING_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
}

