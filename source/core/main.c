#include "gpio.h"

int main(void)
{
    //GPIO输入
    //配置P34为输入，P34外接SW1按键
    Gpio_InitIO(3, 4, GpioDirIn);
    //如果按键SW1按下，为低电平，则跳出循环继续执行程序
    while (TRUE == Gpio_GetIO(3,4));   

    //GPIO输出
    //初始化外部GPIO P33为输出、上拉、开漏，P33端口外接LED2
    Gpio_InitIOExt(3, 3, GpioDirOut, TRUE, FALSE, TRUE, FALSE);
    //设置GPIO值（翻转）
    while (1)
    {
        //输出高电平，LED2灭
        Gpio_SetIO(3, 3, TRUE);
        delay1ms(1000);

        //输出低电平，LED2亮
        Gpio_SetIO(3, 3, FALSE);
        delay1ms(1000);
    }
}
