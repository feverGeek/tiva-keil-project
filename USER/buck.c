/*
 * DC_DC_Buck Demo Application
 * Copyright (c) 2013-2014 China JiLiang University
 * All Rights Reserved.
 */

/*!
 * @file        main.c
 * @version     1.0
 * @author      Kai Zhang
 * @date        2013-07-22
 * @brief       DC_DC_Buck_Tiva 生成2个PWM信号， 频率500Hz
 * PWM1			PC4 输出 可以通过滚轮调节PWM的占空比 占空比调节范围 5 ~ 95 %
 * PWM2			PA6 输出 可以通过滚轮调节PWM的占空比 占空比调节范围 5 ~ 95 %
 * PB0			使能PC4调节占空比
 * PE4			使能PA6调节占空比
 * 电流ADC		PB5
 */

#ifndef TARGET_IS_BLIZZARD_RB1
#define TARGET_IS_BLIZZARD_RB1
#endif

#ifndef  PART_TM4C123GH6PM
#define  PART_TM4C123GH6PM
#endif

#include <stdbool.h>
#include <stdint.h>
#include "rom.h"
#include "adc.h"
#include "sysctl.h"
#include "pwm.h"
#include "gpio.h"
#include "pin_map.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "LCDDriver.h"
#include "hw_gpio.h"
#include "interrupt.h"

#define GPIO_PC4_M0PWM6         0x00021004
#define GPIO_PA6_M1PWM2         0x00001805

/************************************************************************
 * 初始化ADC获取相应值
 * 		————|
 *				|
 *		M4	 PB5|<--ADC1：current		模数转换信号源
 *		________|
 *
 ***********************************************************************/

//#define ADC_BASE			ADC0_BASE			// 使用ADC0
#define Current_Sequence 	1					// 使用序列2

void Init_ADC()

{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);					//使能ADC外设

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);				//使能外设GPIOB

	ROM_GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);				//配置PB5为AD所用

	ROM_ADCSequenceConfigure(ADC_BASE, Current_Sequence, ADC_TRIGGER_PROCESSOR, 0);	//ADC序列配置，采样序列1，处理其触发

	ROM_ADCSequenceStepConfigure(ADC_BASE, Current_Sequence, 0, ADC_CTL_CH11 | ADC_CTL_IE |	//序列Step配置，采样序列1，Step0
								 ADC_CTL_END);												//使用通道11(PB5<-->CH11)，采样结束并且产生一个中断信号

	ROM_ADCSequenceEnable(ADC_BASE, Current_Sequence);				//使能采样序列

	ROM_ADCIntClear(ADC_BASE, Current_Sequence);					//清除中断标志
}

/************************************************************************
 *  @berif	初始化PWM获取两组反向的脉宽调制信号
 *  @param	none
 *  @return none
 * 		————|
 *				|
 * 		M4	 PC4|-->M0PWM6(SYS_PWM1)	---------Channel 1
 * 		M4	 PA6|-->M1PWM2(EN_2)		---------Channel 2
 *		________|
 *
 ***********************************************************************/
#define PERIOD_TIME			0x4E0 * 20

void Init_PWM()
{
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);					//设置PWM时钟，1分频

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);			//使能外设PWM0模块
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);			//使能外设PWM1模块

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);			//使能PWM1使用的外设GPIOA
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);			//使能PWM0使用的外设GPIOC

	GPIOPinConfigure(GPIO_PC4_M0PWM6);					//配置PC4复用功能是PWM
	GPIOPinConfigure(GPIO_PA6_M1PWM2);					//配置PA6复用功能为PWM

	GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4);	//配置PC4为PWM0模块使用
	GPIOPinTypePWM(GPIO_PORTA_BASE, GPIO_PIN_6);	//配置PA6为PWM1模式使用

	PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);		//配置PWM0模块，PWM0生成器3，向下计数并且立即更新参数
	PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);		//配置PWM1模块，PWM1生成器1，向下计数并且立即更新参数

	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PERIOD_TIME);			//设置PC4（MOPWM6）产生的PWM周期
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, PERIOD_TIME);			//设置PA6（M1PWM1）产生的PWM周期

	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, PERIOD_TIME / 4);		//设置PC4（MOPWM6）的占空比25%
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, PERIOD_TIME / 4);		//设置PA6（M1PWM1）的占空比25%

	PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT, true);				//使能输出
	PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT, true);				//使能输出

	PWMGenEnable(PWM0_BASE, PWM_GEN_3);							//启动PWM0发生器3的定时器
	PWMGenEnable(PWM1_BASE, PWM_GEN_1);							//启动PWM1发生器1的定时器

	PWMSyncTimeBase(PWM0_BASE, PWM_GEN_3);						//使能发生器模块计数器同步
	PWMSyncTimeBase(PWM1_BASE, PWM_GEN_1);						//使能发生器模块计数器同步

}

/************************************************************
 * @brief  	对端口C、D进行按键中断初始化
 * @param	none
 * @param	none
 ***********************************************************/
void Init_Int_Key() {

	// Make PD6/7 an output.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);


	GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA,
				GPIO_PIN_TYPE_STD_WPU);

	GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_LOW_LEVEL);

	GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_7);

	IntEnable(INT_GPIOC);

	IntMasterEnable();

	GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);

}

void Int_GPIO_C_Handler(void) //中断处理子函数
{
	unsigned long ulStatus;
	int32_t status = 0;
	ulStatus = GPIOIntStatus(GPIO_PORTC_BASE, true); // 读取中断状态
	GPIOIntClear(GPIO_PORTC_BASE, ulStatus); // 清除中断状态，重要
	if (ulStatus & GPIO_PIN_7) // 如果KEY的中断状态有效
	{
		SysCtlDelay(10 * (SysCtlClockGet() / 3000)); // 延时约10ms，消除按键抖动
		while (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00); // 等待KEY抬起
		SysCtlDelay(10 * (SysCtlClockGet()  / 3000)); // 延时约10ms，消除松键抖动

		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
		status = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
		if(!status)
		{
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

		}else{
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
		}

	}

	//HighSpeed_VCA模块使用按键控制
	//VCA_BUTTON_UP_DOWM = 0x01;
}


/********************************
 * 主函数
 * *******************************/
int main(void)
{
	uint32_t pui32ADC_CurrentValue;			// 保存ADC采样值
	uint32_t pui32ADCWhell_Value;			// 保存ADC采样值

	unsigned long ADC_CurrentValue_Old = 0;			// 保留上一次的ADC数值
	unsigned long ADCWhell_Value_Old = 0; 				// 保留上一次的ADC数值

	uint32_t cur_Duty, old_Duty = 0;		 // 根据滚轮ADC转换值换算出当前的时间周期值

	unsigned long Current_Sample;
	unsigned char number = '0';
	int i;
	unsigned char data[4] = {0};
	unsigned long ADCWheel_Sample;
	int32_t status = 0;
	
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |			//配置系统时钟
			SYSCTL_XTAL_16MHZ);

	LCD_Enable();						// 使能LCD
	LCD_Init();							// 初始化LCD
	LCD_ScreenClr();						// 清屏

	Init_Int_Key();

	//LCD默认显示值
	LCD_Draw_Chinese(1, 0, 0);			// 电
	LCD_Draw_Chinese(3, 0, 16);			// 流
	LCD_Draw_Char(':', 0, 30);			// :

	LCD_Draw_Char('C', 0, 0 * 8);
	LCD_Draw_Char('u', 0, 1 * 8);
	LCD_Draw_Char('r', 0, 2 * 8);
	LCD_Draw_Char('r', 0, 3 * 8);
	LCD_Draw_Char('e', 0, 4 * 8);
	LCD_Draw_Char('n', 0, 5 * 8);
	LCD_Draw_Char('t', 0, 6 * 8);
	LCD_Draw_Char('_', 0, 7 * 8);
	LCD_Draw_Char('I', 0, 8 * 8);
	LCD_Draw_Char('n', 0, 9 * 8);
	LCD_Draw_Char(':', 0, 10 * 8);
	LCD_Draw_Char('m', 1, 127-16);			// m
	LCD_Draw_Char('A', 1, 127-8);			// A


	LCD_Draw_Char('D', 2, 0 * 8);
	LCD_Draw_Char('u', 2, 1 * 8);
	LCD_Draw_Char('t', 2, 2 * 8);
	LCD_Draw_Char('y', 2, 3 * 8);
	LCD_Draw_Char(':', 2, 4 * 8);
	LCD_Draw_Char('%', 2, 9 * 8);

	Init_ADCWheel();						//初始化滚轮AD采样模块
	Init_ADC();								//初始化电流采样模块

	Init_PWM();								// 初始化PWM


	while(1)
	{
		//---------------------------------------------------------------------------------
		ADCProcessorTrigger(ADC_BASE, Current_Sequence);				//处理器触发采样


		while(!ADCIntStatus(ADC_BASE, Current_Sequence, false))		//等待采样结束
		{
		}
		ADCIntClear(ADC_BASE, Current_Sequence);						//清除中断标志
		ADCSequenceDataGet(ADC_BASE, Current_Sequence, &pui32ADC_CurrentValue);		//读取采样值

		//观察原理图:INA213将信号扩大50倍，故在R14两端的电流为Current/4096*3.3/50/0.1*1000mA
		//=(current*3.3*1000)/(50*0.1)
		Current_Sample = (pui32ADC_CurrentValue * 660) / 4096;
	
		number = '0';
		i = 0 ;
		data[0] = Current_Sample / 1000;				//采样值根据LCD显示方式拆分
		data[1] = (Current_Sample / 100) % 10;
		data[2] = (Current_Sample / 10) % 10;
		data[3] = (Current_Sample / 1) % 10;

		// 如果ADC采样值发生变化则更新显示
		if(Current_Sample != ADC_CurrentValue_Old)
		{
			for(i = 0; i < 4; ++i)
			{
					LCD_Draw_Char(number + data[i], 1, 50 + 10 * i);
			}

			ADC_CurrentValue_Old = Current_Sample;
		}

		//-----------------------------------------------------------------------------------------------
		ADCWheel_ValueGet(&pui32ADCWhell_Value);							//获取ADCWheel采样值
		ADCWheel_Sample = pui32ADCWhell_Value;
		data[0] = ADCWheel_Sample / 1000;
		data[1] = (ADCWheel_Sample / 100) % 10;
		data[2] = (ADCWheel_Sample / 10) % 10;
		data[3] = (ADCWheel_Sample / 1) % 10;

		// 如果ADC采样值发生变化则更新显示
		if(ADCWheel_Sample != ADCWhell_Value_Old)
		{
			for(i = 0; i < 4; ++i)
			{
				LCD_Draw_Char(number + data[i], 3, 50 + 10 * i);
			}
			ADCWhell_Value_Old = ADCWheel_Sample;
		}

		cur_Duty = 5 + (90 * pui32ADCWhell_Value) / 4096;					//根据采样值计算占空比

		LCD_Draw_Char(number + cur_Duty / 10, 2, 6 * 8);
		LCD_Draw_Char(number + cur_Duty % 10, 2, 7 * 8);

		if((cur_Duty - old_Duty) > 1 || (old_Duty - cur_Duty) > 1)			//占空比发送变化时，更新PWM参数
		{

			uint32_t period = cur_Duty * PERIOD_TIME / 100;

			PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PERIOD_TIME);			//调节PC4端口初始的PWM的占空比
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, period);
			PWMSyncTimeBase(PWM0_BASE, PWM_GEN_3_BIT);

			PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, PERIOD_TIME);			//调节PA6端口输出的PWM的占空比
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, period);
			PWMSyncTimeBase(PWM1_BASE, PWM_GEN_1_BIT);

			old_Duty = cur_Duty;
		}

		status = 0;
		status = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
		if(status)
		{
			LCD_Draw_Char('E', 3, 0 * 8);
			LCD_Draw_Char('N', 3, 1 * 8);
			LCD_Draw_Char('1', 3, 2 * 8);

			LCD_Draw_Char('O', 3, 4 * 8);
			LCD_Draw_Char('N', 3, 5 * 8);
			LCD_Draw_Char(' ', 3, 6 * 8);
		}else{
			LCD_Draw_Char('E', 3, 0 * 8);
			LCD_Draw_Char('N', 3, 1 * 8);
			LCD_Draw_Char('1', 3, 2 * 8);
			LCD_Draw_Char('O', 3, 4 * 8);
			LCD_Draw_Char('F', 3, 5 * 8);
			LCD_Draw_Char('F', 3, 6 * 8);
		}

		ROM_SysCtlDelay(SysCtlClockGet() / 3);				//延时1s
	}
}
//---------------------------------------------------------------------------
