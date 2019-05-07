#include "key.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
//*******************
//#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~
/*______________
 * 							|
 * 			 		 PF4|-->SW1
 *	TIVA		 PF0|-->SW2
 *______________|
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~
void key_scan_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	//~~~~~~~~~~~~~~~~~~~~~~~~~
	//PF0在使用GPIO时先要解锁，然后设置成GPIO，因为这个引脚是NMI管脚 P28 
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;//解锁命令寄存器后要重新锁定防止错误的改变了设置
	//解锁PD7
	HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) |= GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE+GPIO_O_CR) |= (1<<7);//使能以下4个寄存器可写
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) &=(~(1<<7));//清零此寄存器
	HWREG(GPIO_PORTD_BASE+GPIO_O_PDR) &= (~(1<<7));
	HWREG(GPIO_PORTD_BASE+GPIO_O_PUR) &= (~(1<<7));
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) &=(~(1<<7));
	//~~~~~~~~~~~~~~~~~~~~~~~~~
	//所有按键设置为2mA驱动，推挽弱上拉//GPIOPadConfigSet要配合GPIODirModeSet使用
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_DIR_MODE_IN);//将GPIO引脚设置为上拉输入	
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	
	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2|GPIO_PIN_3,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	
	GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint8_t key_scan(void)
{
	if(KEY1==0||KEY2==0)
	{		
		SysCtlDelay(SysCtlClockGet()/300);//10ms
		if		 (KEY1==0)return SW1;//PF4
		else if(KEY2==0)return SW2;//PF0
	}
	return 0;// 无按键按下
}
