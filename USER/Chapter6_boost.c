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
 * @brief       DC-Motor_PWM_Tiva ����1��PWM�źţ�����ֱ�������ת
 * 				PB4 PB7����һ��ΪPWM�źţ�һ��Ϊ�ߵ�ƽ
 * 				����PD6����ʵ��PB4��PB7���ź��л�������ʵ�ֵ������ת��ת
 * 				����ͨ�����ֵ���PWM��ռ�ձ� ռ�ձȵ��ڷ�Χ 5 ~ 95 %
 *
 */
  
#ifndef TARGET_IS_BLIZZARD_RA1
#define TARGET_IS_BLIZZARD_RA1
#endif

#ifndef  PART_TM4C123GH6PM
#define  PART_TM4C123GH6PM
#endif

/*****************************************************
 * PWM_Tiva ����4��PWM�źţ�
 * 4��PWM ÿ·�ź���λ������90���ƫ�ơ�
 * ��·PWM�źŵ�ռ�ձ��ȶ���25%
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
 * ��ʼ��ADC��ȡ���ֵ�ѹֵ
 * 		����������������|
//				|
//		M4	 PE0|<--ADC		ģ��ת���ź�Դ
//		________|
 *
 ***********************************************************************/
#define ADC_BASE		ADC0_BASE			// ʹ��ADC0
#define SequenceNum 	3					// ʹ������3
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
 * ��ʼ��PWM��ȡ���鷴�����������ź�
 * 		����������������|
//				|
 * 		M4	 PB6|-->M0PWM0			---------Channel 1
 * 		M4	 PB7|-->M0PWM1
//		________|
 *
 ***********************************************************************/
#define PERIOD_TIME			12500 / 60    	// 20K Hz	//DC_motor
											// 60K Hz DC Source
// ���������С����ʱ��   Ƶ����200~1000֮��
#define MAX_PERIOD			PERIOD_TIME * 90 / 100
#define MIN_PERIOD			PERIOD_TIME * 12 /  100
void Init_PWM()
{
	// ����PWMʱ�Ӻ�ϵͳʱ��һ��
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

	 // ʹ��PWM����
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

	// ʹ������˿�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	//���ö�Ӧ�ܽŵ�PWM�źŹ���
	GPIOPinConfigure(GPIO_PB4_M0PWM2);

	//����PWM�źŶ˿�
	GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);

	//PWM����������
	PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	//����PWM�ź�����
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PERIOD_TIME);

	//����PWM�ź�ռ�ձ�
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PERIOD_TIME / 10);

	// ʹ��PWM����˿�
	PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);

	// ʹ��PWM������
	PWMGenEnable(PWM0_BASE, PWM_GEN_1);

	// ʹ��PWm������ģ��ļ�ʱ����.
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
 * @brief	�����źŴ����жϺ�����Ӧ��ͨ����Ƶ����Ƶ������ֵ����������ת��
 * @param	null
 * @return  null
 *********************************************************************/
uint32_t old_tick , cur_tick= 0;			// ��¼ǰ�κ͵�ǰ��ϵͳ����ֵ���ڼ����ж���Ӧ״��
uint32_t tick_delay = 0;					// ����ǰ�������жϵ�ʱ���������ڼ�����Ҷ�ֵ�Ƶ��
float cur_frequency , old_frequency= 0.0;	// ��¼ǰ�κ͵�ǰ�ĵ��Ҷ�ֵ�Ƶ��
void Timer3AIntHandler(void)
{
	 unsigned long ulstatus;

	// ��ȡ�жϱ�־λ
	ulstatus = TimerIntStatus(TIMER3_BASE, TIMER_CAPA_MATCH);

	if(ulstatus == TIMER_CAPA_MATCH)
	{
		cur_tick = ROM_SysTickValueGet();

		// ����жϱ�־λ
		TimerIntClear(TIMER3_BASE, ulstatus);

		// ��Ϊ���������Զ�ֹͣ��������Ҫ�������ü���ģ��
		TimerEnable(TIMER3_BASE, TIMER_A);

		// ͳ�ƴ�������
		//OpticalCount ++;
		if(old_tick > cur_tick)
			tick_delay = old_tick - cur_tick;
		old_tick = cur_tick;
	}

}

/**********************************************************************
 * @brief	 Ƶ�ʳ�ʼ��Timer3AΪ���ش���������������ͳ����·�����ź�֮���ʱ���
 * 			��������Ҷ����ת����
 * @param	null
 * @return  null
 *********************************************************************/
void Init_Timer()
{
	 // ����Timer4ģ��
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

		// ����GPIO_M��Ϊ���岶׽��
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

		// ����GPIO��Ϊʹ��Timer4��׽ģʽ
		GPIOPinConfigure(GPIO_PB2_T3CCP0);
		GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);

		// Ϊ�ܽ�����������ģʽ
		GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

		// ����ʹ��Timer4��TimerAģ��Ϊ���ش���������ģʽ
		TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT);

		// ʹ���½��ش���
		TimerControlEvent(TIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);

		// ���ü�����ΧΪ0x8FFF~0X8FFA
		TimerLoadSet(TIMER3_BASE, TIMER_A, 0x4);
		TimerMatchSet(TIMER3_BASE, TIMER_A, 0x0);

		// ע���жϴ���������Ӧ�����¼�
		TimerIntRegister(TIMER3_BASE, TIMER_A, Timer3AIntHandler);

		// ϵͳ���жϿ�
		IntMasterEnable();

		// ʱ���ж������ж��¼�ΪCaptureģʽ�б��ش�������������Ԥ��ֵ
		TimerIntEnable(TIMER3_BASE, TIMER_CAPA_MATCH);

		// NVIC������ʱ��Aģ���ж�
		IntEnable(INT_TIMER3A);

		// ������׽ģ��
		TimerEnable(TIMER3_BASE, TIMER_A);
}

/*****************************************************************************
 * @brief	 ֱ���������_��ȡ�������תƵ��
 * @param	null
 * @return  float	Ƶ��
*****************************************************************************/

float DC_MOTOR_Fre_Get(){

	// ϵͳ��Ƶ ���������жϵ�ϵͳ����ֵ = Ƶ��
	cur_frequency = (1.0*TIVA_MAIN_FREQUENCY) / tick_delay;

	// ������Ƶ��������ȫһ��ʱ����ʾû���ٳ����жϣ���Ƶ��Ӧ��Ϊ0
	if(cur_frequency == old_frequency){
		old_frequency = cur_frequency;
		return 0.0;
	}else{
		old_frequency = cur_frequency;
		return cur_frequency;}


}

/********************************
 * ������
 * *******************************/
int main(void)
{
	uint32_t pui32ADC0Value[1];			// ����ADC����ֵ

	uint32_t cur_Duty, old_Duty; // ���ݹ���ADCת��ֵ�������ǰ��ʱ������ֵ

	cur_Duty = 0;
	old_Duty = 0;

	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);

	// ����ϵͳ��ʱ��
	ROM_SysTickPeriodSet(TIVA_MAIN_FREQUENCY);
	ROM_SysTickEnable();

	//���ֳ�ʼ��
	Init_ADCWheel();

	// ��ʼ��PWM
	Init_PWM();

	// PB7�͵�ƽ
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);

	// �������ʹ���ź�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

	// ��ʼ����ʱ�����ڲ�������ź�
	Init_Timer();

	// ʹ��LCD
	LCD_Enable();

	// ��ʼ��LCD
	LCD_Init();

	// ����
	LCD_ScreenClr();


	// ��ʾ��rpm ���ת�ٵ�λ ת/�֡��ַ�
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

			// ����ռ�ձ�
			PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PERIOD_TIME);
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, period);
			PWMSyncTimeBase(PWM0_BASE, PWM_GEN_2_BIT);
			old_Duty = cur_Duty;
		}

		//����Ƶ�ʲ���ʾ
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

		// ���ADC����ֵ�����仯�������ʾ
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
