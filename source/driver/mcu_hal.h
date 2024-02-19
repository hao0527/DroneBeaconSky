#ifndef MCU_HAL_H_
#define MCU_HAL_H_

#include "base_types.h"

// 系统外设初始化函数
void Mcu_Hal_SystemInit(void);
// GPIO外部中断初始化
void Mcu_Hal_SPL06_GPIOIntInit(void);

#endif /* MCU_HAL_H_ */
