#ifndef SPL0601_H_
#define SPL0601_H_

#include "base_types.h"

/**************************************************************************
 *用户配置宏定义
 **************************************************************************/

#define SPL06_001_EN_FIFO	(1)

/**************************************************************************
 *宏定义
 **************************************************************************/

// 气压测量速率(sample/sec),Background 模式使用
#define PM_RATE_1   (0 << 4)    // 1 measurements pr. sec.
#define PM_RATE_2   (1 << 4)    // 2 measurements pr. sec.
#define PM_RATE_4   (2 << 4)    // 4 measurements pr. sec.
#define PM_RATE_8   (3 << 4)    // 8 measurements pr. sec.
#define PM_RATE_16  (4 << 4)    // 16 measurements pr. sec.
#define PM_RATE_32  (5 << 4)    // 32 measurements pr. sec.
#define PM_RATE_64  (6 << 4)    // 64 measurements pr. sec.
#define PM_RATE_128 (7 << 4)    // 128 measurements pr. sec.

// 气压重采样速率(times),Background 模式使用
#define PM_PRC_1   0    // Sigle         kP=524288   ,3.6ms
#define PM_PRC_2   1    // 2 times       kP=1572864  ,5.2ms
#define PM_PRC_4   2    // 4 times       kP=3670016  ,8.4ms
#define PM_PRC_8   3    // 8 times       kP=7864320  ,14.8ms
#define PM_PRC_16  4    // 16 times      kP=253952   ,27.6ms
#define PM_PRC_32  5    // 32 times      kP=516096   ,53.2ms
#define PM_PRC_64  6    // 64 times      kP=1040384  ,104.4ms
#define PM_PRC_128 7    // 128 times     kP=2088960  ,206.8ms

// 温度测量速率(sample/sec),Background 模式使用
#define TMP_RATE_1   (0 << 4)    // 1 measurements pr. sec.
#define TMP_RATE_2   (1 << 4)    // 2 measurements pr. sec.
#define TMP_RATE_4   (2 << 4)    // 4 measurements pr. sec.
#define TMP_RATE_8   (3 << 4)    // 8 measurements pr. sec.
#define TMP_RATE_16  (4 << 4)    // 16 measurements pr. sec.
#define TMP_RATE_32  (5 << 4)    // 32 measurements pr. sec.
#define TMP_RATE_64  (6 << 4)    // 64 measurements pr. sec.
#define TMP_RATE_128 (7 << 4)    // 128 measurements pr. sec.

// 温度重采样速率(times),Background 模式使用
#define TMP_PRC_1   0    // Sigle
#define TMP_PRC_2   1    // 2 times
#define TMP_PRC_4   2    // 4 times
#define TMP_PRC_8   3    // 8 times
#define TMP_PRC_16  4    // 16 times
#define TMP_PRC_32  5    // 32 times
#define TMP_PRC_64  6    // 64 times
#define TMP_PRC_128 7    // 128 times

// SPL06_MEAS_CFG
#define MEAS_COEF_RDY   0x80
#define MEAS_SENSOR_RDY 0x40    // 传感器初始化完成
#define MEAS_TMP_RDY    0x20    // 有新的温度数据
#define MEAS_PRS_RDY    0x10    // 有新的气压数据

#define MEAS_CTRL_Standby             0x00    // 空闲模式
#define MEAS_CTRL_PressMeasure        0x01    // 单次气压测量
#define MEAS_CTRL_TempMeasure         0x02    // 单次温度测量
#define MEAS_CTRL_ContinuousPress     0x05    // 连续气压测量
#define MEAS_CTRL_ContinuousTemp      0x06    // 连续温度测量
#define MEAS_CTRL_ContinuousPressTemp 0x07    // 连续气压温度测量

// FIFO_STS
#define SPL06_FIFO_FULL  0x02
#define SPL06_FIFO_EMPTY 0x01

// INT_STS
#define SPL06_INT_FIFO_FULL 0x04
#define SPL06_INT_TMP       0x02
#define SPL06_INT_PRS       0x01

// CFG_REG
#define SPL06_CFG_T_SHIFT 0x08    // oversampling times>8时必须使用
#define SPL06_CFG_P_SHIFT 0x04

#define SPL06_PSR_B2 0x00    // 气压值
#define SPL06_PSR_B1 0x01
#define SPL06_PSR_B0 0x02
#define SPL06_TMP_B2 0x03    // 温度值
#define SPL06_TMP_B1 0x04
#define SPL06_TMP_B0 0x05

#define SPL06_PSR_CFG  0x06    // 气压测量配置
#define SPL06_TMP_CFG  0x07    // 温度测量配置
#define SPL06_MEAS_CFG 0x08    // 测量模式配置

#define SPL06_CFG_REG  0x09
#define SPL06_INT_STS  0x0A
#define SPL06_FIFO_STS 0x0B

#define SPL06_RESET 0x0C
#define SPL06_ID    0x0D

#define SPL06_COEF      0x10    //-0x21
#define SPL06_COEF_SRCE 0x28

#define SPL06DeviceAdd (0x76 << 1)

/**************************************************************************
 *结构体定义
 **************************************************************************/
/*SPL06-001 校准参数*/
typedef struct {
	int16_t C0;
	int16_t C1;
	int32_t C00;
	int32_t C10;
	int16_t C01;
	int16_t C11;
	int16_t C20;
	int16_t C21;
	int16_t C30;

	float kT;
	float kP;
} SPL06_CalibPara_t;

/*气压计实例*/
typedef struct {
	SPL06_CalibPara_t claibPara;
	boolean_t initSuccess;
	float temperture;
	float pressure;
	float altitude;
} SPL06_Info_t;

/**************************************************************************
 *全局变量申明
 **************************************************************************/
extern SPL06_Info_t g_spl06Info;

/**************************************************************************
 *外部函数申明
 **************************************************************************/
void SPL06_Init(void);

#if SPL06_001_EN_FIFO == 1
void SPL06_ReadFIFO(void);
#else
void ReadSPL06_UpdateOnce(void);
#endif

#endif /* SPL0601_H_ */
