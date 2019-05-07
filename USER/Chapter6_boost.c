/*
 * DC-Motor Driver Demo Application
 * Copyright (c) 2013-2014 China JiLiang University
 * All Rights Reserved.
 */

/*!
 * @file        main.c
 * @version     1.0
 * @author      Kai Zhang
 * @date        2013-07-21
 * @brief       DC-Motor_PWM_Tiva 生成1个PWM信号，驱动直流电机旋转
 * 				PB4 PB7其中一个为PWM信号，一个为高电平
 * 				按键PD6可以实现PB4和PB7的信号切换。可以实现电机的正转反转
 * 				可以通过滚轮调节PWM的占空比 占空比调节范围 5 ~ 95 %
 *
 */
  
#ifndef TARGET_IS_BLIZZARD_RA1
#define TARGET_IS_BLIZZARD_RA1
#endif

#ifndef  PART_TM4C123GH6PM
#define  PART_TM4C123GH6PM
#endif

/*****************************************************
 * PWM_Tiva 生成4个PWM信号，
 * 4个PWM 每路信号相位正好有90°的偏移。
 * 四路PWM信号的占空比稳定在25%
 * ***************************************************/
#include <stdbool.h>
#include <stdint.h>
//#include "LCDDriver.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "LCDDriver.h"

#define		TIVA_MAIN_FREQUENCY		12500000

/************************************************************************
 * 初始化ADC获取滚轮电压值
 * 		————————|
//				|
//		M4	 PE0|<--ADC		模数转换信号源
//		________|
 *
 ***********************************************************************/
#define ADC_BASE		ADC0_BASE			// 使用ADC0
#define SequenceNum 	3					// 使用序列3
void Init_ADCWheel(){

	// The ADC0 peripheral must be enabled for use.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	// For this example ADC0 is used with AIN0 on port E7.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	// Select the analog ADC function for these pins.
	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

	// Enable sample sequence 3 with a processor signal trigger.  Sequence 3
	ROM_ADCSequenceConfigure(ADC_BASE, SequenceNum, ADC_TRIGGER_PROCESSOR, 0);

	// Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
	ROM_ADCSequenceStepConfigure(ADC_BASE, SequenceNum, 0, ADC_CTL_CH3 | ADC_CTL_IE |
							 ADC_CTL_END);

	// Since sample sequence 3 is now configured, it must be enabled.
	ROM_ADCSequenceEnable(ADC_BASE, SequenceNum);

	// Clear the interrupt status flag.  This is done to make sure the
	ROM_ADCIntClear(ADC_BASE, SequenceNum);
}


/************************************************************************
 * 初始化PWM获取两组反响的脉宽调制信号
 * 		————————|
//				|
 * 		M4	 PB6|-->M0PWM0			---------Channel 1
 * 		M4	 PB7|-->M0PWM1
//		________|
 *
 ***********************************************************************/
#define PERIOD_TIME			12500 / 60    	// 20K Hz	//DC_motor
											// 60K Hz DC Source
// 定义最大最小周期时间   频率在200~1000之间
#define MAX_PERIOD			PERIOD_TIME * 90 / 100
#define MIN_PERIOD			PERIOD_TIME * 12 /  100
void Init_PWM()
{
	// 设置PWM时钟和系统时钟一致
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

	 // 使能PWM外设
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

	// 使能外设端口
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	//设置对应管脚的PWM信号功能
	GPIOPinConfigure(GPIO_PB4_M0PWM2);

	//设置PWM信号端口
	GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);

	//PWM生成器配置
	PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	//设置PWM信号周期
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PERIOD_TIME);

	//设置PWM信号占空比
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PERIOD_TIME / 10);

	// 使能PWM输出端口
	PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);

	// 使能PWM生成器
	PWMGenEnable(PWM0_BASE, PWM_GEN_1);

	// 使能PWm生成器模块的及时功能.
	PWMSyncTimeBase(PWM0_BASE, PWM_GEN_1);

}

bool director = true;
void ChangeDirection()
{
	if(director)
	{
		GPIOPinConfigure(GPIO_PB5_M0PWM3);
		GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_5);
		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, PERIOD_TIME / 4);
		PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true);

		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
	}else
	{

		GPIOPinConfigure(GPIO_PB4_M0PWM2);
		GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);
		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PERIOD_TIME / 4);
		PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);

		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
	}
}

/**********************************************************************
 * @brief	光耦信号触发中断函数响应，通过主频和主频计数差值换算出电机的转速
 * @param	null
 * @return  null
 *********************************************************************/
uint32_t old_tick , cur_tick= 0;			// 记录前次和当前的系统计数值用于鉴别中断响应状况
uint32_t tick_delay = 0;					// 计算前后两次中断的时间间隔，用于计算电机叶轮的频率
float cur_frequency , old_frequency= 0.0;	// 记录前次和当前的电机叶轮的频率
void Timer3AIntHandler(void)
{
	 unsigned long ulstatus;

	// 读取中断标志位
	ulstatus = TimerIntStatus(TIMER3_BASE, TIMER_CAPA_MATCH);

	if(ulstatus == TIMER_CAPA_MATCH)
	{
		cur_tick = ROM_SysTickValueGet();

		// 清除中断标志位
		TimerIntClear(TIMER3_BASE, ulstatus);

		// 因为减计数会自动停止，所以需要重新启用计数模块
		TimerEnable(TIMER3_BASE, TIMER_A);

		// 统计触发次数
		//OpticalCount ++;
		if(old_tick > cur_tick)
			tick_delay = old_tick - cur_tick;
		old_tick = cur_tick;
	}

}

/**********************************************************************
 * @brief	 频率初始化Timer3A为边沿触发减计数，采用统计两路光耦信号之间的时间差
 * 			来折算电机叶轮旋转数度
 * @param	null
 * @return  null
 *********************************************************************/
void Init_Timer()
{
	 // 启用Timer4模块
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

		// 启用GPIO_M作为脉冲捕捉脚
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

		// 配置GPIO脚为使用Timer4捕捉模式
		GPIOPinConfigure(GPIO_PB2_T3CCP0);
		GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);

		// 为管脚配置弱上拉模式
		GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

		// 配置使用Timer4的TimerA模块为边沿触发减计数模式
		TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT);

		// 使用下降沿触发
		TimerControlEvent(TIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);

		// 设置计数范围为0x8FFF~0X8FFA
		TimerLoadSet(TIMER3_BASE, TIMER_A, 0x4);
		TimerMatchSet(TIMER3_BASE, TIMER_A, 0x0);

		// 注册中断处理函数以响应触发事件
		TimerIntRegister(TIMER3_BASE, TIMER_A, Timer3AIntHandler);

		// 系统总中断开
		IntMasterEnable();

		// 时钟中断允许，中断事件为Capture模式中边沿触发，计数到达预设值
		TimerIntEnable(TIMER3_BASE, TIMER_CAPA_MATCH);

		// NVIC中允许定时器A模块中断
		IntEnable(INT_TIMER3A);

		// 启动捕捉模块
		TimerEnable(TIMER3_BASE, TIMER_A);
}

/*****************************************************************************
 * @brief	 直流电机试验_获取电机的旋转频率
 * @param	null
 * @return  float	频率
*****************************************************************************/

float DC_MOTOR_Fre_Get(){

	// 系统主频 除以两次中断的系统计数值 = 频率
	cur_frequency = (1.0*TIVA_MAIN_FREQUENCY) / tick_delay;

	// 当两次频率连续完全一致时，表示没有再出发中断，则频率应该为0
	if(cur_frequency == old_frequency){
		old_frequency = cur_frequency;
		return 0.0;
	}else{
		old_frequency = cur_frequency;
		return cur_frequency;}


}

/********************************
 * 主函数
 * *******************************/
int main(void)
{
	uint32_t pui32ADC0Value[1];			// 保存ADC采样值

	uint32_t cur_Duty, old_Duty; // 根据滚轮ADC转换值换算出当前的时间周期值

	cur_Duty = 0;
	old_Duty = 0;

	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);

	// 启动系统计时器
	ROM_SysTickPeriodSet(TIVA_MAIN_FREQUENCY);
	ROM_SysTickEnable();

	//滚轮初始化
	Init_ADCWheel();

	// 初始化PWM
	Init_PWM();

	// PB7低电平
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);

	// 驱动板的使能信号
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

	// 初始化定时器用于捕获光耦信号
	Init_Timer();

	// 使能LCD
	LCD_Enable();

	// 初始化LCD
	LCD_Init();

	// 清屏
	LCD_ScreenClr();


	// 显示“rpm 电机转速单位 转/分”字符
	LCD_Draw_Char('S', 0, 0);
	LCD_Draw_Char('p', 0, 8);
	LCD_Draw_Char('e', 0, 16);
	LCD_Draw_Char('e', 0, 24);
	LCD_Draw_Char('d', 0, 32);
	LCD_Draw_Char(':', 0, 40);

	LCD_Draw_Char('(', 1, 127-40);
	LCD_Draw_Char('r', 1, 127-32);
	LCD_Draw_Char('/', 1, 127-24);
	LCD_Draw_Char('s', 1, 127-16);
	LCD_Draw_Char(')', 1, 127-8);

	while(1)
	{
		ADCProcessorTrigger(ADC_BASE, SequenceNum);

		// Wait for conversion to be completed.
		while(!ADCIntStatus(ADC_BASE, SequenceNum, false))
		{
		}

		// Clear the ADC interrupt flag.
		ADCIntClear(ADC_BASE, SequenceNum);

		// Read ADC Value.
		ADCSequenceDataGet(ADC_BASE, SequenceNum, pui32ADC0Value);

		cur_Duty = 5 + (95 * pui32ADC0Value[0]) / 4096;

		if((cur_Duty - old_Duty) > 1 || (old_Duty - cur_Duty) > 1)
		{

			uint32_t period = cur_Duty * PERIOD_TIME / 100;

			// 调整占空比
			PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PERIOD_TIME);
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, period);
			PWMSyncTimeBase(PWM0_BASE, PWM_GEN_2_BIT);
			old_Duty = cur_Duty;
		}

		//计算频率并显示
		unsigned char number = '0';
		int i = 0 ;
		unsigned char data[6] = {0};

		int temp= (int)DC_MOTOR_Fre_Get();

		data[0] = temp / 100000;
		data[1] = (temp / 10000) % 10;
		data[2] = (temp / 1000) % 10;
		data[3] = (temp / 100) % 10;
		data[4] = (temp / 10) % 10;
		data[5] = (temp / 1) % 10;

		// 如果ADC采样值发生变化则更新显示
		for(i = 0; i < 6; ++i)
		{
			LCD_Draw_Char(number + data[i], 1, 20 + 10 * i);
		}

		// The function delay (in cycles) = 3 * parameter.  Delay
		// 250 * 4ms arbitrarily.
		SysCtlDelay(SysCtlClockGet() / 6);
	}
}
//---------------------------------------------------------------------------
