#include "lpuart_hal.h"
#include "lpuart.h"

void LpUart_Hal_Init(void)
{
	stc_lpuart_config_t stcConfig;
	stc_lpuart_irq_cb_t stcLPUartIrqCb;
	stc_lpuart_multimode_t stcMulti;
	stc_lpuart_sclk_sel_t stcLpuart_clk;
	stc_lpuart_mode_t stcRunMode;

	DDL_ZERO_STRUCT(stcConfig);
	DDL_ZERO_STRUCT(stcLPUartIrqCb);
	DDL_ZERO_STRUCT(stcMulti);

	Clk_SetPeripheralGate(ClkPeripheralLpUart, TRUE);    // 使能LPUART时钟

	// 通道端口配置
	Gpio_InitIOExt(2, 5, GpioDirOut, TRUE, FALSE, FALSE, FALSE);
	Gpio_InitIOExt(2, 6, GpioDirOut, TRUE, FALSE, FALSE, FALSE);
	Gpio_SetFunc_UART2RX_P25();
	Gpio_SetFunc_UART2TX_P26();

	stcLpuart_clk.enSclk_sel = LPUart_Rcl;
	stcLpuart_clk.enSclk_Prs = LPUartDiv1;
	stcConfig.pstcLpuart_clk = &stcLpuart_clk;

	stcRunMode.enLpMode = LPUartLPMode;    // 正常工作模式或低功耗工作模式配置
	stcRunMode.enMode = LPUartMode1;
	stcConfig.pstcRunMode = &stcRunMode;

	stcLPUartIrqCb.pfnRxIrqCb = NULL;
	stcLPUartIrqCb.pfnTxIrqCb = NULL;
	stcLPUartIrqCb.pfnRxErrIrqCb = NULL;
	stcConfig.pstcIrqCb = &stcLPUartIrqCb;
	stcConfig.bTouchNvic = TRUE;

	stcMulti.enMulti_mode = LPUartNormal;    // 只有模式2/3才有多主机模式
	stcConfig.pstcMultiMode = &stcMulti;

	LPUart_Init(&stcConfig);
	LPUart_EnableFunc(LPUartRx);

	LPUart_ClrStatus(LPUartRxFull);
	LPUart_EnableIrq(LPUartRxIrq);
	EnableNvic(LPUART_IRQn, DDL_IRQ_LEVEL_DEFAULT - 1, TRUE);    // 优先级要比气压计INT高，防止iic阻塞uart rx
}
