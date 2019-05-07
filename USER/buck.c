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
 * @brief       DC_DC_Buck_Tiva ����2��PWM�źţ� Ƶ��500Hz
 * PWM1			PC4 ��� ����ͨ�����ֵ���PWM��ռ�ձ� ռ�ձȵ��ڷ�Χ 5 ~ 95 %
 * PWM2			PA6 ��� ����ͨ�����ֵ���PWM��ռ�ձ� ռ�ձȵ��ڷ�Χ 5 ~ 95 %
 * PB0			ʹ��PC4����ռ�ձ�
 * PE4			ʹ��PA6����ռ�ձ�
 * ����ADC		PB5
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
 * ��ʼ��ADC��ȡ��Ӧֵ
 * 		��������|
 *				|
 *		M4	 PB5|<--ADC1��current		ģ��ת���ź�Դ
 *		________|
 *
 ***********************************************************************/

//#define ADC_BASE			ADC0_BASE			// ʹ��ADC0
#define Current_Sequence 	1					// ʹ������2

void Init_ADC()

{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);					//ʹ��ADC����

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);				//ʹ������GPIOB

	ROM_GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);				//����PB5ΪAD����

	ROM_ADCSequenceConfigure(ADC_BASE, Current_Sequence, ADC_TRIGGER_PROCESSOR, 0);	//ADC�������ã���������1�������䴥��

	ROM_ADCSequenceStepConfigure(ADC_BASE, Current_Sequence, 0, ADC_CTL_CH11 | ADC_CTL_IE |	//����Step���ã���������1��Step0
								 ADC_CTL_END);												//ʹ��ͨ��11(PB5<-->CH11)�������������Ҳ���һ���ж��ź�

	ROM_ADCSequenceEnable(ADC_BASE, Current_Sequence);				//ʹ�ܲ�������

	ROM_ADCIntClear(ADC_BASE, Current_Sequence);					//����жϱ�־
}

/************************************************************************
 *  @berif	��ʼ��PWM��ȡ���鷴�����������ź�
 *  @param	none
 *  @return none
 * 		��������|
 *				|
 * 		M4	 PC4|-->M0PWM6(SYS_PWM1)	---------Channel 1
 * 		M4	 PA6|-->M1PWM2(EN_2)		---------Channel 2
 *		________|
 *
 ***********************************************************************/
#define PERIOD_TIME			0x4E0 * 20

void Init_PWM()
{
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);					//����PWMʱ�ӣ�1��Ƶ

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);			//ʹ������PWM0ģ��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);			//ʹ������PWM1ģ��

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);			//ʹ��PWM1ʹ�õ�����GPIOA
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);			//ʹ��PWM0ʹ�õ�����GPIOC

	GPIOPinConfigure(GPIO_PC4_M0PWM6);					//����PC4���ù�����PWM
	GPIOPinConfigure(GPIO_PA6_M1PWM2);					//����PA6���ù���ΪPWM

	GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4);	//����PC4ΪPWM0ģ��ʹ��
	GPIOPinTypePWM(GPIO_PORTA_BASE, GPIO_PIN_6);	//����PA6ΪPWM1ģʽʹ��

	PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);		//����PWM0ģ�飬PWM0������3�����¼��������������²���
	PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);		//����PWM1ģ�飬PWM1������1�����¼��������������²���

	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PERIOD_TIME);			//����PC4��MOPWM6��������PWM����
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, PERIOD_TIME);			//����PA6��M1PWM1��������PWM����

	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, PERIOD_TIME / 4);		//����PC4��MOPWM6����ռ�ձ�25%
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, PERIOD_TIME / 4);		//����PA6��M1PWM1����ռ�ձ�25%

	PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT, true);				//ʹ�����
	PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT, true);				//ʹ�����

	PWMGenEnable(PWM0_BASE, PWM_GEN_3);							//����PWM0������3�Ķ�ʱ��
	PWMGenEnable(PWM1_BASE, PWM_GEN_1);							//����PWM1������1�Ķ�ʱ��

	PWMSyncTimeBase(PWM0_BASE, PWM_GEN_3);						//ʹ�ܷ�����ģ�������ͬ��
	PWMSyncTimeBase(PWM1_BASE, PWM_GEN_1);						//ʹ�ܷ�����ģ�������ͬ��

}

/************************************************************
 * @brief  	�Զ˿�C��D���а����жϳ�ʼ��
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

void Int_GPIO_C_Handler(void) //�жϴ����Ӻ���
{
	unsigned long ulStatus;
	int32_t status = 0;
	ulStatus = GPIOIntStatus(GPIO_PORTC_BASE, true); // ��ȡ�ж�״̬
	GPIOIntClear(GPIO_PORTC_BASE, ulStatus); // ����ж�״̬����Ҫ
	if (ulStatus & GPIO_PIN_7) // ���KEY���ж�״̬��Ч
	{
		SysCtlDelay(10 * (SysCtlClockGet() / 3000)); // ��ʱԼ10ms��������������
		while (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00); // �ȴ�KEY̧��
		SysCtlDelay(10 * (SysCtlClockGet()  / 3000)); // ��ʱԼ10ms�������ɼ�����

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

	//HighSpeed_VCAģ��ʹ�ð�������
	//VCA_BUTTON_UP_DOWM = 0x01;
}


/********************************
 * ������
 * *******************************/
int main(void)
{
	uint32_t pui32ADC_CurrentValue;			// ����ADC����ֵ
	uint32_t pui32ADCWhell_Value;			// ����ADC����ֵ

	unsigned long ADC_CurrentValue_Old = 0;			// ������һ�ε�ADC��ֵ
	unsigned long ADCWhell_Value_Old = 0; 				// ������һ�ε�ADC��ֵ

	uint32_t cur_Duty, old_Duty = 0;		 // ���ݹ���ADCת��ֵ�������ǰ��ʱ������ֵ

	unsigned long Current_Sample;
	unsigned char number = '0';
	int i;
	unsigned char data[4] = {0};
	unsigned long ADCWheel_Sample;
	int32_t status = 0;
	
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |			//����ϵͳʱ��
			SYSCTL_XTAL_16MHZ);

	LCD_Enable();						// ʹ��LCD
	LCD_Init();							// ��ʼ��LCD
	LCD_ScreenClr();						// ����

	Init_Int_Key();

	//LCDĬ����ʾֵ
	LCD_Draw_Chinese(1, 0, 0);			// ��
	LCD_Draw_Chinese(3, 0, 16);			// ��
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

	Init_ADCWheel();						//��ʼ������AD����ģ��
	Init_ADC();								//��ʼ����������ģ��

	Init_PWM();								// ��ʼ��PWM


	while(1)
	{
		//---------------------------------------------------------------------------------
		ADCProcessorTrigger(ADC_BASE, Current_Sequence);				//��������������


		while(!ADCIntStatus(ADC_BASE, Current_Sequence, false))		//�ȴ���������
		{
		}
		ADCIntClear(ADC_BASE, Current_Sequence);						//����жϱ�־
		ADCSequenceDataGet(ADC_BASE, Current_Sequence, &pui32ADC_CurrentValue);		//��ȡ����ֵ

		//�۲�ԭ��ͼ:INA213���ź�����50��������R14���˵ĵ���ΪCurrent/4096*3.3/50/0.1*1000mA
		//=(current*3.3*1000)/(50*0.1)
		Current_Sample = (pui32ADC_CurrentValue * 660) / 4096;
	
		number = '0';
		i = 0 ;
		data[0] = Current_Sample / 1000;				//����ֵ����LCD��ʾ��ʽ���
		data[1] = (Current_Sample / 100) % 10;
		data[2] = (Current_Sample / 10) % 10;
		data[3] = (Current_Sample / 1) % 10;

		// ���ADC����ֵ�����仯�������ʾ
		if(Current_Sample != ADC_CurrentValue_Old)
		{
			for(i = 0; i < 4; ++i)
			{
					LCD_Draw_Char(number + data[i], 1, 50 + 10 * i);
			}

			ADC_CurrentValue_Old = Current_Sample;
		}

		//-----------------------------------------------------------------------------------------------
		ADCWheel_ValueGet(&pui32ADCWhell_Value);							//��ȡADCWheel����ֵ
		ADCWheel_Sample = pui32ADCWhell_Value;
		data[0] = ADCWheel_Sample / 1000;
		data[1] = (ADCWheel_Sample / 100) % 10;
		data[2] = (ADCWheel_Sample / 10) % 10;
		data[3] = (ADCWheel_Sample / 1) % 10;

		// ���ADC����ֵ�����仯�������ʾ
		if(ADCWheel_Sample != ADCWhell_Value_Old)
		{
			for(i = 0; i < 4; ++i)
			{
				LCD_Draw_Char(number + data[i], 3, 50 + 10 * i);
			}
			ADCWhell_Value_Old = ADCWheel_Sample;
		}

		cur_Duty = 5 + (90 * pui32ADCWhell_Value) / 4096;					//���ݲ���ֵ����ռ�ձ�

		LCD_Draw_Char(number + cur_Duty / 10, 2, 6 * 8);
		LCD_Draw_Char(number + cur_Duty % 10, 2, 7 * 8);

		if((cur_Duty - old_Duty) > 1 || (old_Duty - cur_Duty) > 1)			//ռ�ձȷ��ͱ仯ʱ������PWM����
		{

			uint32_t period = cur_Duty * PERIOD_TIME / 100;

			PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PERIOD_TIME);			//����PC4�˿ڳ�ʼ��PWM��ռ�ձ�
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, period);
			PWMSyncTimeBase(PWM0_BASE, PWM_GEN_3_BIT);

			PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, PERIOD_TIME);			//����PA6�˿������PWM��ռ�ձ�
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

		ROM_SysCtlDelay(SysCtlClockGet() / 3);				//��ʱ1s
	}
}
//---------------------------------------------------------------------------
