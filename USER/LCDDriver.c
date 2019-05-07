#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "LCDDriver.h"
#include "LCD_Matrix.h"
#include "ssi.h"
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "gpio.h"
#include "sysctl.h"
#include "pin_map.h"
#include "adc.h"

/*****************************************************************************************
 *     _________
 * 				|
 * 			 PD0|-->SCK		����ʱ��
 *			 PD3|-->SDA		���ݴ���
 *	TIVA  	 PB1|-->CS		�͵�ƽƬѡ
 *			 PC6|-->CD		����/ָ��Ĵ���
 *			 PE5|-->RST		�͵�ƽ��λ
 *	   _________|
 *
 *******************************************************************************************/
// �͵�ƽƬѡ
#define setCS  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);	//PB1
#define rstCS  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);

// �Ĵ���ѡ���ź�
#define setCD  GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6);	//PC6
#define rstCD  GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);

// �͵�ƽ��λ����λ��ɺ󣬻ص��ߵ�ƽ��Һ��ģ�鿪ʼ����
#define setRES GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);	//PE5
#define rstRES GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);

/******************************************************************************
 * @brief  ����������������е���ʾ����
 * @param
 * @return none
 ******************************************************************************/
void LCD_ScreenClr() {
	unsigned char i, j;
	for (i = 0; i < 9; i++)				//ѭ����0~8ҳ��
	{
		rstCS
		LCD_TransferCmd(0xb0 + i);		//ѡ��ҳ��
		LCD_TransferCmd(0x10);			//ѡ���С�
		LCD_TransferCmd(0x00);
		for (j = 0; j < 132; j++) {
			LCD_TransferData(0x00);		//��������0x00����ʾ0x00
		}
	}
}

/******************************************************************************
 * @brief  Һ������˿ڳ�ʼ��
 * @param
 * @return none
 ******************************************************************************/
void LCD_Enable()
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);					//�����˿�ʹ��
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
//	-----------------------------------------------------------------------------
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);				//�����˿�����Ϊ�����PB1->CS
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);				//PC6->CD
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);				//PE5->RST

//	-----------------------------------------------------------------------------

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);						//ϵͳ����ʹ��,���ó�SSIģʽ
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	ROM_GPIOPinConfigure(GPIO_PD0_SSI3CLK);								//PD0��SSICLK���ܣ�SSIʱ����
	ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);								//PD3��SSITX���ܣ�SSI���ݷ�����

	ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_3);			//SSI�˿ڹ���ʹ��

	ROM_SSIConfigSetExpClk(SSI3_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 50000, 8);	//SSI3,�˿�ģʽ:50K,8λ���ݣ�
    																										//SSI_FRF_MOTO_MODE_0����ģʽ

	ROM_SSIEnable(SSI3_BASE);												//ʹ��SSI3����
//  -----------------------------------------------------------------------------
}

/******************************************************************************
 * @brief  �ϳ�ʱ����ʱ
 * @param
 * @return none
 ******************************************************************************/
//void LCD_Delay2(unsigned int i)
void LCD_Delay_LongTime(unsigned int LongTime) {
	int j, k;
	for (j = 0; j < LongTime; j++)
		for (k = 0; k < 990; k++)
			;
}

/******************************************************************************
 * @brief  �϶�ʱ����ʱ
 * @param
 * @return none
 ******************************************************************************/
//void LCD_Delay3(unsigned int i)
void LCD_Delay_ShortTime(unsigned int ShortTime){
	int j, k;
	for (j = 0; j < ShortTime; j++)
		for (k = 0; k < 10; k++)
			;
}



/******************************************************************************
 * @brief  	��ʾ8*8�ľ�������
 * @param	data ���������ʾ����ָ��
 * @param	type ���Ƿ���Ҫ������ʾ
 * @return  none
 ******************************************************************************/
void LCD_TransferData(unsigned char data) {

	rstCS
	setCD

    while(ROM_SSIBusy(SSI3_BASE));
	ROM_SSIDataPut(SSI3_BASE, data);
    while(ROM_SSIBusy(SSI3_BASE));
}

/******************************************************************************
 * @brief  	����Һ����������
 * @param	command �������ַ�
 * @return  none
 ******************************************************************************/
void LCD_TransferCmd(unsigned char command) {

	rstCS
	rstCD

    while(ROM_SSIBusy(SSI3_BASE));
	ROM_SSIDataPut(SSI3_BASE, command);
    while(ROM_SSIBusy(SSI3_BASE));
}

/******************************************************************************
 * @brief  	��ʼ��Һ��
 * @param	none
 * @return  none
 ******************************************************************************/
void LCD_Init() {
	rstRES
	LCD_Delay_LongTime(50);                 //delay
	setRES
	LCD_Delay_LongTime(50);                 //delay
	LCD_TransferCmd(0xe2); 			/*��λ*/
	LCD_TransferCmd(0x2c); 			/*��ѹ����1*/
	LCD_TransferCmd(0x2e); 			/*��ѹ����2*/
	LCD_TransferCmd(0x2f); 			/*��ѹ����3*/
	LCD_TransferCmd(0x23); 			/*�ֵ��Աȶȣ������÷�Χ20��27*/
	LCD_TransferCmd(0x81);			/*΢���Աȶ�*/
	LCD_TransferCmd(0x28); 			/*΢���Աȶȵ�ֵ�������÷�Χ0��63*/
	LCD_TransferCmd(0xa2); 			/*1/9 ƫѹ�ȣ�bias��*/
	LCD_TransferCmd(0xc8); 			/*��ɨ��˳�򣺴��ϵ���c0 c8 */
	LCD_TransferCmd(0xa0); 			/*��ɨ��˳�򣺴����� a0 a1*/
	LCD_TransferCmd(0xa6); 			/*���� 0xa6������ 0xa7*/
	LCD_TransferCmd(0xaf); 			/*����ʾ*/

}


/******************************************************************************
 * @brief  	��Һ����8*8��ģ������˳ʱ��90�ȵ���ת��������Һ������ʾģʽ
 * @param	c	��Ҫ��ʾ�����ֻ��ַ�
 * @param	row	�ַ���ʾ������ ȡֵ��Χ0~3
 * @param	col	�ַ���ʾ������ ȡֵ��Χ0~119
 * @return  0 �� ��ʾ�����������
 *			1�� ��ʾ��ʾ�ɹ�
 ******************************************************************************/
unsigned char LCD_Draw_Char(char c, unsigned char row, unsigned char col) {

	unsigned char * add = asc + (c - 0x20) * 16;
	unsigned int i,j;							//������
	unsigned char col_0 = col;					// ����λ
	unsigned char col_1 = col >> 4;  			// ����λ

	//----------------������-------------
	//--------------����0��ʾ�������---------------
	if (col > LCD_MAX_COL - 1) {
		return 0;
	}
	if (row > LCD_MAX_RAW - 1) {
		return 0;
	}

	for (i = 0; i < 2; i++)
	{
		rstCS
		LCD_TransferCmd(0xb0 + row * 2 + i);            //���á�ҳ��
		LCD_TransferCmd(0x10 + (col_1 & 0x0f));			//���á��С��Լ��еĸ���λ
		LCD_TransferCmd(0x00 + (col_0 & 0x0f));			//���á��С��Լ��еĵ���λ
		for (j = 0; j < 8; j++)
		{
			LCD_TransferData(*add);
			add++;
		}
	}

	return 1; 								//����1��ʾִ�гɹ���
}


/******************************************************************************
 * @brief  	��Һ����8*8��ģ������˳ʱ��90�ȵ���ת��������Һ������ʾģʽ
 * @param	index	��Ҫ��ʾ�ĺ��ֵ���ģ���
 * @param	row	�ַ���ʾ������ ,ȡֵ��Χ0~3
 * @param	col	�ַ���ʾ������ ,ȡֵ��Χ0~119
 * @return  0 �� ��ʾ�����������
 *			1�� ��ʾ��ʾ�ɹ�
 ******************************************************************************/
unsigned char LCD_Draw_Chinese(unsigned char index, unsigned char row,
		unsigned char col) {
	unsigned char * chinese = CHINESE_MATRIX + 32 * index;
	unsigned int i, j;					//������
	unsigned char col_0 = col;		//�е���λ
	unsigned char col_1 = col >> 4;  //�и���λ

	//----------------������-------------
	//-----------����0��ʾ�������------------
	if (col > 111) {
		return 0;
	}
	if (row > 3) {
		return 0;
	}

	for (i = 0; i < 2; i++)
	{
		rstCS
		LCD_TransferCmd(0xb0 + row * 2 + i);             //���á�ҳ��
		LCD_TransferCmd(0x10 + (col_1 & 0x0f));			 //���á��С��Լ��еĸ���λ
		LCD_TransferCmd(0x00 + (col_0 & 0x0f));			 //���á��С��Լ��еĵ���λ
		for (j = 0; j < 16; j++)
		{
			LCD_TransferData(*chinese);
			chinese++;
		}
	}
	return 1; //����1��ʾִ�гɹ���
}

/************************************************************
 * @brief  	�Զ˿�C��D���а�����ʼ��
 * @param	none
 * @return	none
 *      ��������
 * 				|
 *			 PC7|<--Button1
 * 	TIVA	 PD6|<--Button2
 * 			 PD7|<--Button3
 *		________|
 ***********************************************************/
void Init_Key()
{
	//��ʼ������GPIO
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);


	// ����PDΪ2MA���������
	ROM_GPIOPadConfigSet(GPIO_PORTC_BASE,  GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE,  GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE,  GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//����GPIO����ģʽ
	ROM_GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);
	ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_DIR_MODE_IN);
	ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);

	}


/************************************************************************
 * @brief  	����ɨ�躯��
 * @param	none
 * @return	none
 * 		��������
 * 				|
 *			 PC7|<--Button1
 * 	TIVA	 PD6|<--Button2
 * 			 PD7|<--Button3
 *		________|
 *
 *  @return	0x00			û�м�����
 *  		0x01			����PC7��S1
 *  		0x02			����PD6��S2
 *  		0x03            ����PD7��S3
 ***********************************************************************/
unsigned char scan_key(void)
{

	if (ROM_GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms��������������
		while (ROM_GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00); 		// �ȴ�KEY̧��
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms�������ɼ�����

		// 	do something

		return 0x01;
	}

	if (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms��������������
		while (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) == 0x00); 		// �ȴ�KEY̧��
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms�������ɼ�����

		// 	do something

		return 0x02;
	}

	if (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms��������������
		while (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) == 0x00); 		// �ȴ�KEY̧��
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// ��ʱԼ10ms�������ɼ�����
		return 0x03;
	}

	return 0;
}

/************************************************************************************************
 * @brief  	��ʼ�����ֵ��蹦��ģ��
 * @param	none
 * @return	none
 *      ��������
 *      		|
 * 	Tiva	 PE0|<--ADC		���ֵ���ADC����
 *		________|
 *
 * ���ֵ���������PE0�ڣ�ʹ�øÿڵ�ADC���ܽ��в�������ȡ���ֵ���ֵ
 *
 ************************************************************************************************/
void Init_ADCWheel()
{

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);								//ʹ��ADC0��������

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);							//ʹ��ADC0ʹ�õĶ˿�

	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);							//PE0���ó�ADCģʽ

	ROM_ADCSequenceConfigure(ADC_BASE, ADCWheel_Sequence, ADC_TRIGGER_PROCESSOR, 0);			//ʹ������0������������ADC����

	ROM_ADCSequenceStepConfigure(ADC_BASE, ADCWheel_Sequence, 0, ADC_CTL_CH3 | ADC_CTL_IE |    //ʹ������0������0��ͨ��3��CH3<-->PE0������
								 ADC_CTL_END);									//���Ҳ����жϣ���������

	ROM_ADCSequenceEnable(ADC_BASE, ADCWheel_Sequence);										//ʹ��ADC0��������
	ROM_ADCIntClear(ADC_BASE, ADCWheel_Sequence);												//����жϱ�־
}

/********************************************************************************************************
 * @brief ��ȡ���ֲ���ֵ
 * @param *pui32ADCValue,���ֵ������ֵ��ŵĵ�ַ
 * @return none
 *      ��������
 *      		|
 * 	Tiva	 PE0|<--ADC		���ֵ���ADC����
 *		________|
 ********************************************************************************************************/
void ADCWheel_ValueGet(uint32_t *pui32ADCValue)
{
	ROM_ADCProcessorTrigger(ADC_BASE, ADCWheel_Sequence);				//��������������

	while(!ROM_ADCIntStatus(ADC_BASE, ADCWheel_Sequence, false))		//�ȴ���������
	{
	}

	ROM_ADCIntClear(ADC_BASE, ADCWheel_Sequence);						//����жϱ�־

	ROM_ADCSequenceDataGet(ADC_BASE, ADCWheel_Sequence, pui32ADCValue);	//��ȡ����ֵ
}
