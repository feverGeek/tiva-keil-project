
/**************************************************************************************************
 * LCD Module(液晶模块)包含液晶，按键，滚轮电阻，其端口连接如下图。该驱动库完成LCD Module所有功能
 * 接口的定义。
 *	   _________
 *				|
 *			 PD0|-->SCK		串行时钟
 *			 PD3|-->SDA		数据传输
 *	TIVA  	 PB1|-->CS		低电平片选
 *			 PC6|-->CD		数据/指令寄存器
 *			 PE5|-->RST		低电平复位
 *			 	|
 *			 PC7|<--Button1
 * 			 PD6|<--Button2
 * 			 PD7|<--Button3
 * 			    |
 * 			 PE0|<--ADC		滚轮电阻ADC采样
 *		________|
 *
 *
 *
 ***************************************************************************************************/
#ifndef LCD_H_
#define LCD_H_

//------------------------------------------------------------------------------------------------

#include "hw_memmap.h"
#include "hw_types.h"
#include "gpio.h"
#include "sysctl.h"
#include "rom.h"
#include "rom_map.h"

//---------------------------------------------------------------------------
#define LCD_DRAW_NORMAL				0		// 正常显示
#define LCD_DRAW_HIGHLIGHT				1		// 背光显示
#define LCD_MAX_RAW					4		// 液晶最大行数
#define LCD_MAX_COL					120		// 液晶最大列数
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//枚举变量，液晶行定义
enum
{
	LINE_ONE,
	LINE_TWO,
	LINE_THREE,
	LINE_FOUR,

	LINE_NUM
};

//枚举变量，液晶方向定义
enum
{
	DIRECTOR_RAW,
	DIRECTOR_COL,

	DIRECTOR_NUM
};

enum
{
	SSI1_UNITS = 101,
	SSI1_TENS = 85,
	SSI1_HUNDREDS = 69,
	SSI1_THOUSANDS = 53
};//cursor_define_for_MDAC;

#define CHAR_WIDTH			8				//字符宽度
#define WORD_WIDTH			16				//字宽度
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

/**************************************************************************************************
 *
 * 功能接口定义，所有接口函数如下，包括LCD功能模块函数接口，按键功能模块接口以及滚轮电阻模块功能接口
 * 外加2个延时函数，延时函数为辅助LCD模块。
 *
 **************************************************************************************************/

/*
 * 函数名：LCD_ScreenClr
 * 用法：LCD_ScreenClr();
 * ------------------------------------
 * 用来进行LCD清屏
 */
void LCD_ScreenClr(void);

/*
 * 函数名：LCD_Enable
 * 用法：LCD_Enable();
 * ------------------------------------
 *  使能LCD所用到的端口
 */
void LCD_Enable(void);

/*
 * 函数名：LCD_Delay_LongTime
 * 用法：LCE_Delay_LongTime(LongTime);
 * ------------------------------------------
 * 进行较长时间的延时
 */
void LCD_Delay_LongTime(unsigned int LongTime);

/*
 * 函数名：LCD_Delay_ShortTime
 * 用法：LCD_Delay_ShortTime(ShortTime);
 * -------------------------------------------
 * 进行较短时间的延时
 */
void LCD_Delay_ShortTime(unsigned int ShortTime);

/*
 * 函数名：LCD_TransferData
 * 用法：LCD_TransferData(data);
 * ---------------------------------------
 * 发送数据至LCD，data为字模数据
 */
void LCD_TransferData(unsigned char data);

/*
 * 函数名：LCD_TransferCmd
 * 用法：LCD_TransferCmd(command);
 * --------------------------------------------
 * 发送LCD指令
 */
void LCD_TransferCmd(unsigned char command);

/*
 * 函数名：LCD_Init
 * 用法：LCD_Init();
 * ------------------------------------
 * LCD初始化设置
 */
void LCD_Init(void);

/*
 * 函数名：LCD_Draw_Char
 * 用法：char = LCD_Draw_Char(c,page,col);
 * ---------------------------------------
 * LCD中显示一个字符，8*16的字模大小
 */
unsigned char LCD_Draw_Char(char c,unsigned char page,unsigned char col);

/*
 * 函数名：LCD_Draw_Byte
 * 用法：LCD_Draw_Byte(b_invert);
 * ---------------------------------
 * LCD显示一个字节，8*8的字模大小
 */
void LCD_Draw_Byte(bool b_invert);

/*
 * 函数名：LCD_Draw_Chinese
 * 用法：char = LCD_Draw_Chinese(index,raw,col);
 * ---------------------------------------
 * LCD显示汉字，16*16的字模大小
 */
unsigned char LCD_Draw_Chinese(unsigned char index, unsigned char raw,unsigned char col);

/*
 * 函数名：LCD_matrixChange
 * 用法：LCD_MatriChange(*p_Char);
 * -----------------------------------------
 * LCD矩阵变化
 */
void LCD_MatrixChange(unsigned char * p_Char);

/*
 * 函数名：LCD_InvertLight_All
 * 用法：LCD_InvertLight_All();
 * ---------------------------------------
 * LCD整屏翻转设置
 */
void LCD_InvertLight_All(void);

/*
 * 函数名：DispalyGraphicl
 * 用法：DisplayGraphic1();
 * ----------------------------------
 * 全屏显示图形
 */
void DisplayGraphic1(void);

/*
 * 函数名：Init_Key
 * 用法：Init_Key();
 * ------------------------------------
 * 初始化按键功能模块
 */
void Init_Key(void);

/*
 * 函数名：scan_key
 * 用法：char = scan_key();
 * -----------------------------------
 * 键扫描
 */
unsigned char scan_key(void);


//#define ADC_BASE				ADC0_BASE			//ADC0_BASE
#define ADCWheel_Sequence		0					//ADCWheel序列，序列0，SS0
#define ADCMAX					4096
#define ADCREFVOLTS			3300
/*
 * 函数名：Init_ADCWheel()
 * 用法：Init_ADCWheel();
 * ----------------------------
 * 初始化滚轮电阻功能模块
 */
void Init_ADCWheel(void);

/*
 * 函数名：ADCWheel_ValueGet
 * 用法： ADCWheel_ValueGet(&pui32ADCValue);
 * ----------------------------------------
 * 获取滚轮AD采样值
 */
void  ADCWheel_ValueGet(uint32_t *pui32ADCValue);


#endif
//-------------------------------------------------------------------------------------------------
