#ifndef I2C_HAL_H_
#define I2C_HAL_H_

#include "base_types.h"

// 主发送函数
en_result_t I2C_Hal_MasterWriteData(uint8_t *pu8Data, uint32_t u32Len);
// 主接收函数
en_result_t I2C_Hal_MasterReadData(uint8_t *pu8Data, uint32_t u32Len);
// 初始化函数
void I2C_Hal_Init(uint8_t slaveAddr);

#endif /* I2C_HAL_H_ */
