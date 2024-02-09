#include "spl0601.h"
#include "mcu_hal.h"
#include "lpuart_hal.h"
#include "ddl.h"

int main(void)
{
    Mcu_Hal_SystemInit();
    LpUart_Hal_Init();
    SPL06_Init();
    while (1)
    {
        delay1ms(1000);
    }
}
