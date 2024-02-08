#include "i2c.h"
#include "i2c_hal.h"
#include "spl0601.h"

/**
 ******************************************************************************
 ** \brief  主机接收函数
 **
 ** \param pu8Data读数据存放缓存，u32Len读数据长度
 **
 ** \retval 读数据是否成功
 **
 ******************************************************************************/
en_result_t I2C_Hal_MasterReadData(uint8_t *pu8Data, uint32_t u32Len)
{
	en_result_t enRet = Error;
	uint8_t u8i = 0, u8State;

	I2C_SetFunc(I2cStart_En);

	while (1) {
		while (0 == I2C_GetIrq()) {
		}
		u8State = I2C_GetState();
		switch (u8State) {
		case 0x08:
		case 0x10:
			I2C_ClearFunc(I2cStart_En);
			I2C_WriteByte(I2C_SLAVEADDR | 0x01);    // 从机地址发送OK
			break;
		case 0x40:
			if (u32Len > 1) {
				I2C_SetFunc(I2cAck_En);
			}
			break;
		case 0x50:
			pu8Data[u8i++] = I2C_ReadByte();
			if (u8i == u32Len - 1) {
				I2C_ClearFunc(I2cAck_En);
			}
			break;
		case 0x58:
			pu8Data[u8i++] = I2C_ReadByte();
			I2C_SetFunc(I2cStop_En);
			break;
		case 0x38:
			I2C_SetFunc(I2cStart_En);
			break;
		case 0x48:
			I2C_SetFunc(I2cStop_En);
			I2C_SetFunc(I2cStart_En);
			break;
		default:
			I2C_SetFunc(I2cStart_En);    // 其他错误状态，重新发送起始条件
			break;
		}
		I2C_ClearIrq();
		if (u8i == u32Len) {
			break;
		}
	}
	enRet = Ok;
	return enRet;
}

/**
 ******************************************************************************
 ** \brief  主机发送函数
 **
 ** \param pu8Data写数据，u32Len写数据长度
 **
 ** \retval 写数据是否成功
 **
 ******************************************************************************/
en_result_t I2C_Hal_MasterWriteData(uint8_t *pu8Data, uint32_t u32Len)
{
	en_result_t enRet = Error;
	uint8_t u8i = 0, u8State;
	I2C_SetFunc(I2cStart_En);
	while (1) {
		while (0 == I2C_GetIrq()) {
			;
		}
		u8State = I2C_GetState();
		switch (u8State) {
		case 0x08:
		case 0x10:
			I2C_ClearFunc(I2cStart_En);
			I2C_WriteByte(I2C_SLAVEADDR);    // 从设备地址发送
			break;
		case 0x18:
		case 0x28:
			I2C_WriteByte(pu8Data[u8i++]);
			break;
		case 0x20:
		case 0x38:
			I2C_SetFunc(I2cStart_En);
			break;
		case 0x30:
			I2C_SetFunc(I2cStop_En);
			I2C_SetFunc(I2cStart_En);
			break;
		default:
			break;
		}
		if (u8i > u32Len) {
			I2C_SetFunc(I2cStop_En);    // 此顺序不能调换，出停止条件
			I2C_ClearIrq();
			break;
		}
		I2C_ClearIrq();
		// delay(10000);
	}
	enRet = Ok;
	return enRet;
}

void I2C_Hal_Init(void)
{
	stc_i2c_config_t stcI2cCfg;
	DDL_ZERO_STRUCT(stcI2cCfg);

	Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
	Gpio_SetIO(3, 5, TRUE);
	Gpio_SetIO(3, 6, TRUE);
	Gpio_InitIOExt(3, 5, GpioDirOut, FALSE, FALSE, TRUE, FALSE);
	Gpio_InitIOExt(3, 6, GpioDirOut, FALSE, FALSE, TRUE, FALSE);
	Gpio_SetFunc_I2CDAT_P35();
	Gpio_SetFunc_I2CCLK_P36();

	Clk_SetPeripheralGate(ClkPeripheralI2c, TRUE);
	stcI2cCfg.enFunc = I2cBaud_En;
	stcI2cCfg.u8Tm = 0x01;
	stcI2cCfg.pfnI2cCb = NULL;
	stcI2cCfg.bTouchNvic = FALSE;
	I2C_DeInit();
	I2C_Init(&stcI2cCfg);

	I2C_SetFunc(I2cHlm_En);
	I2C_SetFunc(I2cMode_En);

    if (stcI2cCfg.bTouchNvic == TRUE) {
		EnableNvic(I2C_IRQn, 3, TRUE);
	}
}
