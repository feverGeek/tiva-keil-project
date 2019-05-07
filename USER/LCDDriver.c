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
 * 			 PD0|-->SCK		串行时钟
 *			 PD3|-->SDA		数据传输
 *	TIVA  	 PB1|-->CS		低电平片选
 *			 PC6|-->CD		数据/指令寄存器
 *			 PE5|-->RST		低电平复位
 *	   _________|
 *
 *******************************************************************************************/
// 低电平片选
#define setCS  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);	//PB1
#define rstCS  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);

// 寄存器选择信号
#define setCD  GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6);	//PC6
#define rstCD  GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);

// 低电平复位，复位完成后，回到高电平，液晶模块开始工作
#define setRES GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);	//PE5
#define rstRES GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);

/******************************************************************************
 * @brief  清屏函数，清楚所有的显示内容
 * @param
 * @return none
 ******************************************************************************/
void LCD_ScreenClr() {
	unsigned char i, j;
	for (i = 0; i < 9; i++)				//循环第0~8页面
	{
		rstCS
		LCD_TransferCmd(0xb0 + i);		//选择“页”
		LCD_TransferCmd(0x10);			//选择“列”
		LCD_TransferCmd(0x00);
		for (j = 0; j < 132; j++) {
			LCD_TransferData(0x00);		//输入数据0x00，显示0x00
		}
	}
}

/******************************************************************************
 * @brief  液晶五个端口初始化
 * @param
 * @return none
 ******************************************************************************/
void LCD_Enable()
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);					//各个端口使能
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
//	-----------------------------------------------------------------------------
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);				//各个端口设置为输出，PB1->CS
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);				//PC6->CD
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);				//PE5->RST

//	-----------------------------------------------------------------------------

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);						//系统外设使能,配置成SSI模式
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	ROM_GPIOPinConfigure(GPIO_PD0_SSI3CLK);								//PD0，SSICLK功能，SSI时钟线
	ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);								//PD3，SSITX功能，SSI数据发送线

	ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_3);			//SSI端口功能使能

	ROM_SSIConfigSetExpClk(SSI3_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 50000, 8);	//SSI3,端口模式:50K,8位数据，
    																										//SSI_FRF_MOTO_MODE_0，主模式

	ROM_SSIEnable(SSI3_BASE);												//使能SSI3功能
//  -----------------------------------------------------------------------------
}

/******************************************************************************
 * @brief  较长时间延时
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
 * @brief  较短时间延时
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
 * @brief  	显示8*8的矩阵像素
 * @param	data ：输入的显示数据指针
 * @param	type ：是否需要反相显示
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
 * @brief  	发送液晶控制命令
 * @param	command ：命令字符
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
 * @brief  	初始化液晶
 * @param	none
 * @return  none
 ******************************************************************************/
void LCD_Init() {
	rstRES
	LCD_Delay_LongTime(50);                 //delay
	setRES
	LCD_Delay_LongTime(50);                 //delay
	LCD_TransferCmd(0xe2); 			/*软复位*/
	LCD_TransferCmd(0x2c); 			/*升压步聚1*/
	LCD_TransferCmd(0x2e); 			/*升压步聚2*/
	LCD_TransferCmd(0x2f); 			/*升压步聚3*/
	LCD_TransferCmd(0x23); 			/*粗调对比度，可设置范围20～27*/
	LCD_TransferCmd(0x81);			/*微调对比度*/
	LCD_TransferCmd(0x28); 			/*微调对比度的值，可设置范围0～63*/
	LCD_TransferCmd(0xa2); 			/*1/9 偏压比（bias）*/
	LCD_TransferCmd(0xc8); 			/*行扫描顺序：从上到下c0 c8 */
	LCD_TransferCmd(0xa0); 			/*列扫描顺序：从左到右 a0 a1*/
	LCD_TransferCmd(0xa6); 			/*正显 0xa6，反显 0xa7*/
	LCD_TransferCmd(0xaf); 			/*开显示*/

}


/******************************************************************************
 * @brief  	对液晶的8*8字模矩阵做顺时针90度的旋转，来贴合液晶的显示模式
 * @param	c	需要显示的数字或字符
 * @param	row	字符显示的行数 取值范围0~3
 * @param	col	字符显示的列数 取值范围0~119
 * @return  0 ， 表示输入参数出错
 *			1， 表示显示成功
 ******************************************************************************/
unsigned char LCD_Draw_Char(char c, unsigned char row, unsigned char col) {

	unsigned char * add = asc + (c - 0x20) * 16;
	unsigned int i,j;							//计数器
	unsigned char col_0 = col;					// 低四位
	unsigned char col_1 = col >> 4;  			// 高四位

	//----------------输入检测-------------
	//--------------返回0表示输入出错---------------
	if (col > LCD_MAX_COL - 1) {
		return 0;
	}
	if (row > LCD_MAX_RAW - 1) {
		return 0;
	}

	for (i = 0; i < 2; i++)
	{
		rstCS
		LCD_TransferCmd(0xb0 + row * 2 + i);            //设置“页”
		LCD_TransferCmd(0x10 + (col_1 & 0x0f));			//设置“列”以及列的高四位
		LCD_TransferCmd(0x00 + (col_0 & 0x0f));			//设置“列”以及列的低四位
		for (j = 0; j < 8; j++)
		{
			LCD_TransferData(*add);
			add++;
		}
	}

	return 1; 								//返回1表示执行成功！
}


/******************************************************************************
 * @brief  	对液晶的8*8字模矩阵做顺时针90度的旋转，来贴合液晶的显示模式
 * @param	index	需要显示的汉字的字模编号
 * @param	row	字符显示的行数 ,取值范围0~3
 * @param	col	字符显示的列数 ,取值范围0~119
 * @return  0 ， 表示输入参数出错
 *			1， 表示显示成功
 ******************************************************************************/
unsigned char LCD_Draw_Chinese(unsigned char index, unsigned char row,
		unsigned char col) {
	unsigned char * chinese = CHINESE_MATRIX + 32 * index;
	unsigned int i, j;					//计数器
	unsigned char col_0 = col;		//列低四位
	unsigned char col_1 = col >> 4;  //列高四位

	//----------------输入检测-------------
	//-----------返回0表示输入出错------------
	if (col > 111) {
		return 0;
	}
	if (row > 3) {
		return 0;
	}

	for (i = 0; i < 2; i++)
	{
		rstCS
		LCD_TransferCmd(0xb0 + row * 2 + i);             //设置“页”
		LCD_TransferCmd(0x10 + (col_1 & 0x0f));			 //设置“列”以及列的高四位
		LCD_TransferCmd(0x00 + (col_0 & 0x0f));			 //设置“列”以及列的低四位
		for (j = 0; j < 16; j++)
		{
			LCD_TransferData(*chinese);
			chinese++;
		}
	}
	return 1; //返回1表示执行成功！
}

/************************************************************
 * @brief  	对端口C、D进行按键初始化
 * @param	none
 * @return	none
 *      ————
 * 				|
 *			 PC7|<--Button1
 * 	TIVA	 PD6|<--Button2
 * 			 PD7|<--Button3
 *		________|
 ***********************************************************/
void Init_Key()
{
	//初始化外设GPIO
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);


	// 设置PD为2MA，推挽输出
	ROM_GPIOPadConfigSet(GPIO_PORTC_BASE,  GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE,  GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	ROM_GPIOPadConfigSet(GPIO_PORTD_BASE,  GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//设置GPIO输入模式
	ROM_GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);
	ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_DIR_MODE_IN);
	ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);

	}


/************************************************************************
 * @brief  	按键扫描函数
 * @param	none
 * @return	none
 * 		————
 * 				|
 *			 PC7|<--Button1
 * 	TIVA	 PD6|<--Button2
 * 			 PD7|<--Button3
 *		________|
 *
 *  @return	0x00			没有键按下
 *  		0x01			按下PC7，S1
 *  		0x02			按下PD6，S2
 *  		0x03            按下PD7，S3
 ***********************************************************************/
unsigned char scan_key(void)
{

	if (ROM_GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除按键抖动
		while (ROM_GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) == 0x00); 		// 等待KEY抬起
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除松键抖动

		// 	do something

		return 0x01;
	}

	if (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除按键抖动
		while (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) == 0x00); 		// 等待KEY抬起
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除松键抖动

		// 	do something

		return 0x02;
	}

	if (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) == 0x00)
	{
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除按键抖动
		while (ROM_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) == 0x00); 		// 等待KEY抬起
		ROM_SysCtlDelay(10*(ROM_SysCtlClockGet() / 3000)); 					// 延时约10ms，消除松键抖动
		return 0x03;
	}

	return 0;
}

/************************************************************************************************
 * @brief  	初始化滚轮电阻功能模块
 * @param	none
 * @return	none
 *      ————
 *      		|
 * 	Tiva	 PE0|<--ADC		滚轮电阻ADC采样
 *		________|
 *
 * 滚轮电阻连接至PE0口，使用该口的ADC功能进行采样，获取滚轮电阻值
 *
 ************************************************************************************************/
void Init_ADCWheel()
{

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);								//使能ADC0功能外设

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);							//使能ADC0使用的端口

	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);							//PE0配置成ADC模式

	ROM_ADCSequenceConfigure(ADC_BASE, ADCWheel_Sequence, ADC_TRIGGER_PROCESSOR, 0);			//使能序列0，处理器触发ADC采样

	ROM_ADCSequenceStepConfigure(ADC_BASE, ADCWheel_Sequence, 0, ADC_CTL_CH3 | ADC_CTL_IE |    //使用序列0，步进0，通道3（CH3<-->PE0）采样
								 ADC_CTL_END);									//并且产生中断，结束采样

	ROM_ADCSequenceEnable(ADC_BASE, ADCWheel_Sequence);										//使能ADC0采样序列
	ROM_ADCIntClear(ADC_BASE, ADCWheel_Sequence);												//清除中断标志
}

/********************************************************************************************************
 * @brief 获取滚轮采样值
 * @param *pui32ADCValue,滚轮电阻采样值存放的地址
 * @return none
 *      ————
 *      		|
 * 	Tiva	 PE0|<--ADC		滚轮电阻ADC采样
 *		________|
 ********************************************************************************************************/
void ADCWheel_ValueGet(uint32_t *pui32ADCValue)
{
	ROM_ADCProcessorTrigger(ADC_BASE, ADCWheel_Sequence);				//处理器触发采样

	while(!ROM_ADCIntStatus(ADC_BASE, ADCWheel_Sequence, false))		//等待采样结束
	{
	}

	ROM_ADCIntClear(ADC_BASE, ADCWheel_Sequence);						//清除中断标志

	ROM_ADCSequenceDataGet(ADC_BASE, ADCWheel_Sequence, pui32ADCValue);	//获取采样值
}
