#include "spl0601.h"
#include "i2c_hal.h"


/****************************************************************************
 *  @function :  SPL06ReadRes_Buf
 *  @brief    :  SPL06 I2C 读取多字节函数
 *****************************************************************************/
boolean_t SPL06ReadRes_Buf(uint8_t ResAddr, uint8_t *pData, uint8_t DataLen)
{
	if (Ok != I2C_Hal_MasterWriteData(&ResAddr, 1))
		return FALSE;
	if (Ok != I2C_Hal_MasterReadData(pData, DataLen))
		return FALSE;

	return TRUE;
}

/****************************************************************************
 *  @function :  SPL06ReadRes_Single
 *  @brief    :  SPL06 I2C读取单字节函数
 *****************************************************************************/
uint8_t SPL06ReadRes_Single(uint8_t ResAddr)
{
	uint8_t DataTemp = 0;

	if (Ok != I2C_Hal_MasterWriteData(&ResAddr, 1))
		return 0xff;
	if (Ok != I2C_Hal_MasterReadData(&DataTemp, 1))
		return 0xff;

	return DataTemp;
}

/****************************************************************************
 *  @function :  SPL06WriteRes_Single
 *  @brief    :  SPL06 I2C WriteRes
 *****************************************************************************/
boolean_t SPL06WriteRes_Single(uint8_t ResAddr, uint8_t DataTemp)
{
	uint8_t Temp[2];
	Temp[0] = ResAddr;
	Temp[1] = DataTemp;

	if (Ok != I2C_Hal_MasterWriteData(Temp, 2))
		return FALSE;

	return TRUE;
}

void SPL06_Config_Temperature(uint8_t rate, uint8_t oversampling, T_SPL06_calibPara *ptSPL06_calibPara)
{
	uint8_t temp;
	switch (oversampling) {
	case TMP_PRC_1:
		ptSPL06_calibPara->kT = 524288;
		break;
	case TMP_PRC_2:
		ptSPL06_calibPara->kT = 1572864;
		break;
	case TMP_PRC_4:
		ptSPL06_calibPara->kT = 3670016;
		break;
	case TMP_PRC_8:
		ptSPL06_calibPara->kT = 7864320;
		break;
	case TMP_PRC_16:
		ptSPL06_calibPara->kT = 253952;
		break;
	case TMP_PRC_32:
		ptSPL06_calibPara->kT = 516096;
		break;
	case TMP_PRC_64:
		ptSPL06_calibPara->kT = 1040384;
		break;
	case TMP_PRC_128:
		ptSPL06_calibPara->kT = 2088960;
		break;
	}

	SPL06WriteRes_Single(SPL06_TMP_CFG, rate | oversampling | 0x80);    // 温度每秒128次测量一次
	if (oversampling > TMP_PRC_8) {
		temp = SPL06ReadRes_Single(SPL06_CFG_REG);
		SPL06WriteRes_Single(SPL06_CFG_REG, temp | SPL06_CFG_T_SHIFT);
	}
}

void SPL06_Config_Pressure(uint8_t rate, uint8_t oversampling, T_SPL06_calibPara *ptSPL06_calibPara)
{
	uint8_t temp;
	switch (oversampling) {
	case PM_PRC_1:
		ptSPL06_calibPara->kP = 524288;
		break;
	case PM_PRC_2:
		ptSPL06_calibPara->kP = 1572864;
		break;
	case PM_PRC_4:
		ptSPL06_calibPara->kP = 3670016;
		break;
	case PM_PRC_8:
		ptSPL06_calibPara->kP = 7864320;
		break;
	case PM_PRC_16:
		ptSPL06_calibPara->kP = 253952;
		break;
	case PM_PRC_32:
		ptSPL06_calibPara->kP = 516096;
		break;
	case PM_PRC_64:
		ptSPL06_calibPara->kP = 1040384;
		break;
	case PM_PRC_128:
		ptSPL06_calibPara->kP = 2088960;
		break;
	}

	SPL06WriteRes_Single(SPL06_PSR_CFG, rate | oversampling);
	if (oversampling > PM_PRC_8) {
		temp = SPL06ReadRes_Single(SPL06_CFG_REG);
		SPL06WriteRes_Single(SPL06_CFG_REG, temp | SPL06_CFG_P_SHIFT);
	}
}

void SPL06_Start(uint8_t mode)
{
	SPL06WriteRes_Single(SPL06_MEAS_CFG, mode);
}

/****************************************************************************
	*  @function :  SPL06_Init
	*  @brief    :  SPL06 初始化函数
*****************************************************************************/
void SPL06_Init(T_SPL06_calibPara *ptSPL06_calibPara)
{
	uint8_t coef[18];
	uint8_t id;

	I2C_Hal_Init(SPL06DeviceAdd);	// 初始化I2C

	id = SPL06ReadRes_Single(SPL06_ID);    // ID 正常情况是0x10

	SPL06ReadRes_Buf(SPL06_COEF, coef, 18);
	ptSPL06_calibPara->C0 = ((int16_t)coef[0] << 4) + ((coef[1] & 0xF0) >> 4);
	ptSPL06_calibPara->C0 = (ptSPL06_calibPara->C0 & 0x0800) ? (0xF000 | ptSPL06_calibPara->C0) : ptSPL06_calibPara->C0;
	ptSPL06_calibPara->C1 = ((int16_t)(coef[1] & 0x0F) << 8) + coef[2];
	ptSPL06_calibPara->C1 = (ptSPL06_calibPara->C1 & 0x0800) ? (0xF000 | ptSPL06_calibPara->C1) : ptSPL06_calibPara->C1;
	ptSPL06_calibPara->C00 = ((int32_t)coef[3] << 12) + ((int32_t)coef[4] << 4) + (coef[5] >> 4);
	ptSPL06_calibPara->C00 = (ptSPL06_calibPara->C00 & 0x080000) ? (0xFFF00000 | ptSPL06_calibPara->C00) : ptSPL06_calibPara->C00;
	ptSPL06_calibPara->C10 = ((int32_t)(coef[5] & 0x0F) << 16) + ((int32_t)coef[6] << 8) + coef[7];
	ptSPL06_calibPara->C10 = (ptSPL06_calibPara->C10 & 0x080000) ? (0xFFF00000 | ptSPL06_calibPara->C10) : ptSPL06_calibPara->C10;
	ptSPL06_calibPara->C01 = ((int16_t)coef[8] << 8) + coef[9];
	ptSPL06_calibPara->C11 = ((int16_t)coef[10] << 8) + coef[11];
	ptSPL06_calibPara->C20 = ((int16_t)coef[12] << 8) + coef[13];
	ptSPL06_calibPara->C21 = ((int16_t)coef[14] << 8) + coef[15];
	ptSPL06_calibPara->C30 = ((int16_t)coef[16] << 8) + coef[17];

	SPL06_Config_Pressure(PM_RATE_4, PM_PRC_32, ptSPL06_calibPara);
	SPL06_Config_Temperature(PM_RATE_4, TMP_PRC_8, ptSPL06_calibPara);

	SPL06_Start(MEAS_CTRL_ContinuousPressTemp);    // 启动连续的气压温度测量
}

/****************************************************************************
	*  @function :  SPL06_SoftReset
	*  @brief    :  SPL06软件复位
*****************************************************************************/
void SPL06_SoftReset()
{
	SPL06WriteRes_Single(SPL06_RESET, 0x89);
}

int32_t SPL06_Get_Pressure_Adc()
{
	uint8_t buf[3];
	int32_t adc;

	SPL06ReadRes_Buf(SPL06_PSR_B2, buf, 3);
	adc = (int32_t)(buf[0] << 16) + (int32_t)(buf[1] << 8) + buf[2];
	adc = (adc & 0x800000) ? (0xFF000000 | adc) : adc;
	return adc;
}

int32_t SPL06_Get_Temperature_Adc()
{
	uint8_t buf[3];
	int32_t adc;

	SPL06ReadRes_Buf(SPL06_TMP_B2, buf, 3);
	adc = (int32_t)(buf[0] << 16) + (int32_t)(buf[1] << 8) + buf[2];
	adc = (adc & 0x800000) ? (0xFF000000 | adc) : adc;
	return adc;
}

float ReadSPL06_Pressure(T_SPL06_calibPara *ptSPL06_calibPara)
{
	float Traw_src, Praw_src;
	float Temp;
	float qua2, qua3;
	int32_t raw_temp, raw_press;

	raw_temp = SPL06_Get_Temperature_Adc();
	raw_press = SPL06_Get_Pressure_Adc();

	Traw_src = raw_temp / ptSPL06_calibPara->kT;
	Praw_src = raw_press / ptSPL06_calibPara->kP;

	// 计算温度
	Temp = 0.5f * ptSPL06_calibPara->C0 + Traw_src * ptSPL06_calibPara->C1;

	// 计算气压
	qua2 = ptSPL06_calibPara->C10 + Praw_src * (ptSPL06_calibPara->C20 + Praw_src * ptSPL06_calibPara->C30);
	qua3 = Traw_src * Praw_src * (ptSPL06_calibPara->C11 + Praw_src * ptSPL06_calibPara->C21);
	return ptSPL06_calibPara->C00 + Praw_src * qua2 + Traw_src * ptSPL06_calibPara->C01 + qua3;
}
