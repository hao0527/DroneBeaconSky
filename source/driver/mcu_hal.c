#include "mcu_hal.h"
#include "clk.h"
#include "gpio.h"

void Mcu_Hal_SystemInit(void)
{
	stc_clk_config_t stcCfg;
	DDL_ZERO_STRUCT(stcCfg);

	// 打开GPIO时钟
	Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);

	// CLK初始化，默认RCH 4M
	stcCfg.enClkSrc = ClkRCH;
	stcCfg.enHClkDiv = ClkDiv1;
	stcCfg.enPClkDiv = ClkDiv1;
	Clk_Init(&stcCfg);

	// Clk_SwitchTo(ClkRCL);
	// Clk_SetRCHFreq(ClkFreq24Mhz);
	// Clk_SwitchTo(ClkRCH);

	// 设置低速时钟为38400
	Clk_SetRCLFreq(ClkFreq38_4K);
}

void Mcu_Hal_SPL06_GPIOIntInit(void)
{
	// 打开GPIO时钟
	Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);

	// 初始化外部IO P33
	Gpio_InitIO(3, 3, GpioDirIn);
	Gpio_InitIOExt(3, 3, GpioDirIn, FALSE, TRUE, FALSE, 0);

	// 开启GPIO中断
	Gpio_ClearIrq(3, 3);
	Gpio_EnableIrq(3, 3, GpioIrqRising);
	EnableNvic(PORT3_IRQn, DDL_IRQ_LEVEL_DEFAULT, TRUE);
}
