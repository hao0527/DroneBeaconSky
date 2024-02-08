#include "spl0601.h"
#include "mcu_hal.h"
#include "ddl.h"

T_SPL06_calibPara t_SPL06_calibPara;
volatile float pressure;

int main(void)
{
    Mcu_Hal_SystemInit();
    SPL06_Init(&t_SPL06_calibPara);
    while (1)
    {
        pressure = ReadSPL06_Pressure(&t_SPL06_calibPara);
        delay1ms(1000);
    }
}
