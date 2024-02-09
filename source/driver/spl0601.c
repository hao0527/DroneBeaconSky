#include <math.h>
#include "spl0601.h"
#include "i2c_hal.h"
#include "mcu_hal.h"
#include "ddl.h"

SPL06_Info_t g_spl06Info;

/****************************************************************************
 *  @function :  SPL06ReadRes_Buf
 *  @brief    :  SPL06 I2C 读取多字节函数
 *****************************************************************************/
static boolean_t SPL06ReadRes_Buf(uint8_t ResAddr, uint8_t *pData, uint8_t DataLen)
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
static uint8_t SPL06ReadRes_Single(uint8_t ResAddr)
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
static boolean_t SPL06WriteRes_Single(uint8_t ResAddr, uint8_t DataTemp)
{
	uint8_t Temp[2];
	Temp[0] = ResAddr;
	Temp[1] = DataTemp;

	if (Ok != I2C_Hal_MasterWriteData(Temp, 2))
		return FALSE;

	return TRUE;
}

static void SPL06_Config_Temperature(uint8_t rate, uint8_t oversampling)
{
	uint8_t temp;
	switch (oversampling) {
	case TMP_PRC_1:
		g_spl06Info.claibPara.kT = 524288;
		break;
	case TMP_PRC_2:
		g_spl06Info.claibPara.kT = 1572864;
		break;
	case TMP_PRC_4:
		g_spl06Info.claibPara.kT = 3670016;
		break;
	case TMP_PRC_8:
		g_spl06Info.claibPara.kT = 7864320;
		break;
	case TMP_PRC_16:
		g_spl06Info.claibPara.kT = 253952;
		break;
	case TMP_PRC_32:
		g_spl06Info.claibPara.kT = 516096;
		break;
	case TMP_PRC_64:
		g_spl06Info.claibPara.kT = 1040384;
		break;
	case TMP_PRC_128:
		g_spl06Info.claibPara.kT = 2088960;
		break;
	}

	SPL06WriteRes_Single(SPL06_TMP_CFG, rate | oversampling | 0x80);    // 温度每秒128次测量一次
	if (oversampling > TMP_PRC_8) {
		temp = SPL06ReadRes_Single(SPL06_CFG_REG);
		SPL06WriteRes_Single(SPL06_CFG_REG, temp | SPL06_CFG_T_SHIFT);
	}
}

static void SPL06_Config_Pressure(uint8_t rate, uint8_t oversampling)
{
	uint8_t temp;
	switch (oversampling) {
	case PM_PRC_1:
		g_spl06Info.claibPara.kP = 524288;
		break;
	case PM_PRC_2:
		g_spl06Info.claibPara.kP = 1572864;
		break;
	case PM_PRC_4:
		g_spl06Info.claibPara.kP = 3670016;
		break;
	case PM_PRC_8:
		g_spl06Info.claibPara.kP = 7864320;
		break;
	case PM_PRC_16:
		g_spl06Info.claibPara.kP = 253952;
		break;
	case PM_PRC_32:
		g_spl06Info.claibPara.kP = 516096;
		break;
	case PM_PRC_64:
		g_spl06Info.claibPara.kP = 1040384;
		break;
	case PM_PRC_128:
		g_spl06Info.claibPara.kP = 2088960;
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
 *  @function :  SPL06_SoftReset
 *  @brief    :  SPL06软件复位
 *****************************************************************************/
static void SPL06_SoftReset(void)
{
	SPL06WriteRes_Single(SPL06_RESET, 0x89);
}

static int32_t SPL06_Get_Pressure_Adc(void)
{
	uint8_t buf[3];
	int32_t adc;

	SPL06ReadRes_Buf(SPL06_PSR_B2, buf, 3);
	adc = (int32_t)(buf[0] << 16) + (int32_t)(buf[1] << 8) + buf[2];
	adc = (adc & 0x800000) ? (0xFF000000 | adc) : adc;
	return adc;
}

#if SPL06_001_EN_FIFO == 1
static void SPL06_Config_FIFO_INT(void)
{
	uint8_t temp;

	temp = SPL06ReadRes_Single(SPL06_CFG_REG);
	temp |= 1 << 7;    // Interrupt (on SDO pin) active level: 1 - Active high
	temp |= 1 << 6;    // Generate interrupt when the FIFO is full: 1 - Enable
	temp |= 1 << 1;    // Enable the FIFO: 1 - Enable
	SPL06WriteRes_Single(SPL06_CFG_REG, temp);
}
#else
static int32_t SPL06_Get_Temperature_Adc(void)
{
	uint8_t buf[3];
	int32_t adc;

	SPL06ReadRes_Buf(SPL06_TMP_B2, buf, 3);
	adc = (int32_t)(buf[0] << 16) + (int32_t)(buf[1] << 8) + buf[2];
	adc = (adc & 0x800000) ? (0xFF000000 | adc) : adc;
	return adc;
}
#endif

/****************************************************************************
 *  @function :  SPL06_Init
 *  @brief    :  SPL06 初始化函数
 *****************************************************************************/
void SPL06_Init(void)
{
	uint8_t coef[18];
	volatile uint8_t id;

#if SPL06_001_EN_FIFO == 1
	Mcu_Hal_SPL06_GPIOIntInit();
	delay1ms(100);
#endif
	I2C_Hal_Init(SPL06DeviceAdd);    // 初始化I2C

	id = SPL06ReadRes_Single(SPL06_ID);    // ID 正常情况是0x10
	if (id != 0x10) {
		g_spl06Info.initSuccess = FALSE;
		return;
	}

	SPL06_SoftReset();
	delay1ms(100);	// 必须要有
	SPL06ReadRes_Buf(SPL06_COEF, coef, 18);
	g_spl06Info.claibPara.C0 = ((int16_t)coef[0] << 4) + ((coef[1] & 0xF0) >> 4);
	g_spl06Info.claibPara.C0 = (g_spl06Info.claibPara.C0 & 0x0800) ? (0xF000 | g_spl06Info.claibPara.C0) : g_spl06Info.claibPara.C0;
	g_spl06Info.claibPara.C1 = ((int16_t)(coef[1] & 0x0F) << 8) + coef[2];
	g_spl06Info.claibPara.C1 = (g_spl06Info.claibPara.C1 & 0x0800) ? (0xF000 | g_spl06Info.claibPara.C1) : g_spl06Info.claibPara.C1;
	g_spl06Info.claibPara.C00 = ((int32_t)coef[3] << 12) + ((int32_t)coef[4] << 4) + (coef[5] >> 4);
	g_spl06Info.claibPara.C00 = (g_spl06Info.claibPara.C00 & 0x080000) ? (0xFFF00000 | g_spl06Info.claibPara.C00) : g_spl06Info.claibPara.C00;
	g_spl06Info.claibPara.C10 = ((int32_t)(coef[5] & 0x0F) << 16) + ((int32_t)coef[6] << 8) + coef[7];
	g_spl06Info.claibPara.C10 = (g_spl06Info.claibPara.C10 & 0x080000) ? (0xFFF00000 | g_spl06Info.claibPara.C10) : g_spl06Info.claibPara.C10;
	g_spl06Info.claibPara.C01 = ((int16_t)coef[8] << 8) + coef[9];
	g_spl06Info.claibPara.C11 = ((int16_t)coef[10] << 8) + coef[11];
	g_spl06Info.claibPara.C20 = ((int16_t)coef[12] << 8) + coef[13];
	g_spl06Info.claibPara.C21 = ((int16_t)coef[14] << 8) + coef[15];
	g_spl06Info.claibPara.C30 = ((int16_t)coef[16] << 8) + coef[17];

	SPL06_Config_Pressure(PM_RATE_1, PM_PRC_2);
	SPL06_Config_Temperature(TMP_RATE_1, TMP_PRC_2);
#if SPL06_001_EN_FIFO == 1
	SPL06_Config_FIFO_INT();
#endif

	SPL06_Start(MEAS_CTRL_ContinuousPressTemp);    // 启动连续的气压温度测量
	g_spl06Info.initSuccess = TRUE;
}

#if SPL06_001_EN_FIFO == 1
void SPL06_ReadFIFO(void)
{
	float Traw_src, Praw_src;
	float qua2, qua3;
	int32_t raw_data, raw_temp_sum = 0, raw_press_sum = 0;
	uint8_t temp_cnt = 0, press_cnt = 0;

	if (g_spl06Info.initSuccess == FALSE)
		return;
	// 读取INT_STS清标志位，并判断是否为INT_FIFO_FULL
	if (!(SPL06ReadRes_Single(SPL06_INT_STS) & 0x4))
		return;

	// 循环直到FIFO_EMPTY
	while(!(SPL06ReadRes_Single(SPL06_FIFO_STS) & 0x1)) {
		raw_data = SPL06_Get_Pressure_Adc();
		if (raw_data & 0x1) {
			// 气压数据
			raw_press_sum += raw_data;
			press_cnt++;
		} else {
			// 温度数据
			raw_temp_sum += raw_data;
			temp_cnt++;
		}
	}

	if (press_cnt == 0 || temp_cnt == 0)
		return;    // 防止除0
	Traw_src = raw_temp_sum / temp_cnt / g_spl06Info.claibPara.kT;
	Praw_src = raw_press_sum / press_cnt / g_spl06Info.claibPara.kP;
	// 计算温度
	g_spl06Info.temperture = 0.5f * g_spl06Info.claibPara.C0 + Traw_src * g_spl06Info.claibPara.C1;
	// 计算气压
	qua2 = g_spl06Info.claibPara.C10 + Praw_src * (g_spl06Info.claibPara.C20 + Praw_src * g_spl06Info.claibPara.C30);
	qua3 = Traw_src * Praw_src * (g_spl06Info.claibPara.C11 + Praw_src * g_spl06Info.claibPara.C21);
	g_spl06Info.pressure = g_spl06Info.claibPara.C00 + Praw_src * qua2 + Traw_src * g_spl06Info.claibPara.C01 + qua3;
	// 计算高度
	g_spl06Info.altitude = (pow((101325 / g_spl06Info.pressure), 1 / 5.257) - 1) * (g_spl06Info.temperture + 273.15) / 0.0065;
}
#else
// 如果使用FIFO，不可以使用此接口更新数据
void ReadSPL06_UpdateOnce(void)
{
	float Traw_src, Praw_src;
	float qua2, qua3;
	int32_t raw_temp, raw_press;

	if (g_spl06Info.initSuccess == FALSE)
		return;

	raw_temp = SPL06_Get_Temperature_Adc();
	raw_press = SPL06_Get_Pressure_Adc();
	Traw_src = raw_temp / g_spl06Info.claibPara.kT;
	Praw_src = raw_press / g_spl06Info.claibPara.kP;
	// 计算温度
	g_spl06Info.temperture = 0.5f * g_spl06Info.claibPara.C0 + Traw_src * g_spl06Info.claibPara.C1;
	// 计算气压
	qua2 = g_spl06Info.claibPara.C10 + Praw_src * (g_spl06Info.claibPara.C20 + Praw_src * g_spl06Info.claibPara.C30);
	qua3 = Traw_src * Praw_src * (g_spl06Info.claibPara.C11 + Praw_src * g_spl06Info.claibPara.C21);
	g_spl06Info.pressure = g_spl06Info.claibPara.C00 + Praw_src * qua2 + Traw_src * g_spl06Info.claibPara.C01 + qua3;
	// 计算高度
	g_spl06Info.altitude = (pow((101325 / g_spl06Info.pressure), 1 / 5.257) - 1) * (g_spl06Info.temperture + 273.15) / 0.0065;
}
#endif
