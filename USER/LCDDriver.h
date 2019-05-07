
/**************************************************************************************************
 * LCD Module(Һ��ģ��)����Һ�������������ֵ��裬��˿���������ͼ�������������LCD Module���й���
 * �ӿڵĶ��塣
 *	   _________
 *				|
 *			 PD0|-->SCK		����ʱ��
 *			 PD3|-->SDA		���ݴ���
 *	TIVA  	 PB1|-->CS		�͵�ƽƬѡ
 *			 PC6|-->CD		����/ָ��Ĵ���
 *			 PE5|-->RST		�͵�ƽ��λ
 *			 	|
 *			 PC7|<--Button1
 * 			 PD6|<--Button2
 * 			 PD7|<--Button3
 * 			    |
 * 			 PE0|<--ADC		���ֵ���ADC����
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
#define LCD_DRAW_NORMAL				0		// ������ʾ
#define LCD_DRAW_HIGHLIGHT				1		// ������ʾ
#define LCD_MAX_RAW					4		// Һ���������
#define LCD_MAX_COL					120		// Һ���������
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ö�ٱ�����Һ���ж���
enum
{
	LINE_ONE,
	LINE_TWO,
	LINE_THREE,
	LINE_FOUR,

	LINE_NUM
};

//ö�ٱ�����Һ��������
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

#define CHAR_WIDTH			8				//�ַ����
#define WORD_WIDTH			16				//�ֿ��
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

/**************************************************************************************************
 *
 * ���ܽӿڶ��壬���нӿں������£�����LCD����ģ�麯���ӿڣ���������ģ��ӿ��Լ����ֵ���ģ�鹦�ܽӿ�
 * ���2����ʱ��������ʱ����Ϊ����LCDģ�顣
 *
 **************************************************************************************************/

/*
 * ��������LCD_ScreenClr
 * �÷���LCD_ScreenClr();
 * ------------------------------------
 * ��������LCD����
 */
void LCD_ScreenClr(void);

/*
 * ��������LCD_Enable
 * �÷���LCD_Enable();
 * ------------------------------------
 *  ʹ��LCD���õ��Ķ˿�
 */
void LCD_Enable(void);

/*
 * ��������LCD_Delay_LongTime
 * �÷���LCE_Delay_LongTime(LongTime);
 * ------------------------------------------
 * ���нϳ�ʱ�����ʱ
 */
void LCD_Delay_LongTime(unsigned int LongTime);

/*
 * ��������LCD_Delay_ShortTime
 * �÷���LCD_Delay_ShortTime(ShortTime);
 * -------------------------------------------
 * ���н϶�ʱ�����ʱ
 */
void LCD_Delay_ShortTime(unsigned int ShortTime);

/*
 * ��������LCD_TransferData
 * �÷���LCD_TransferData(data);
 * ---------------------------------------
 * ����������LCD��dataΪ��ģ����
 */
void LCD_TransferData(unsigned char data);

/*
 * ��������LCD_TransferCmd
 * �÷���LCD_TransferCmd(command);
 * --------------------------------------------
 * ����LCDָ��
 */
void LCD_TransferCmd(unsigned char command);

/*
 * ��������LCD_Init
 * �÷���LCD_Init();
 * ------------------------------------
 * LCD��ʼ������
 */
void LCD_Init(void);

/*
 * ��������LCD_Draw_Char
 * �÷���char = LCD_Draw_Char(c,page,col);
 * ---------------------------------------
 * LCD����ʾһ���ַ���8*16����ģ��С
 */
unsigned char LCD_Draw_Char(char c,unsigned char page,unsigned char col);

/*
 * ��������LCD_Draw_Byte
 * �÷���LCD_Draw_Byte(b_invert);
 * ---------------------------------
 * LCD��ʾһ���ֽڣ�8*8����ģ��С
 */
void LCD_Draw_Byte(bool b_invert);

/*
 * ��������LCD_Draw_Chinese
 * �÷���char = LCD_Draw_Chinese(index,raw,col);
 * ---------------------------------------
 * LCD��ʾ���֣�16*16����ģ��С
 */
unsigned char LCD_Draw_Chinese(unsigned char index, unsigned char raw,unsigned char col);

/*
 * ��������LCD_matrixChange
 * �÷���LCD_MatriChange(*p_Char);
 * -----------------------------------------
 * LCD����仯
 */
void LCD_MatrixChange(unsigned char * p_Char);

/*
 * ��������LCD_InvertLight_All
 * �÷���LCD_InvertLight_All();
 * ---------------------------------------
 * LCD������ת����
 */
void LCD_InvertLight_All(void);

/*
 * ��������DispalyGraphicl
 * �÷���DisplayGraphic1();
 * ----------------------------------
 * ȫ����ʾͼ��
 */
void DisplayGraphic1(void);

/*
 * ��������Init_Key
 * �÷���Init_Key();
 * ------------------------------------
 * ��ʼ����������ģ��
 */
void Init_Key(void);

/*
 * ��������scan_key
 * �÷���char = scan_key();
 * -----------------------------------
 * ��ɨ��
 */
unsigned char scan_key(void);


//#define ADC_BASE				ADC0_BASE			//ADC0_BASE
#define ADCWheel_Sequence		0					//ADCWheel���У�����0��SS0
#define ADCMAX					4096
#define ADCREFVOLTS			3300
/*
 * ��������Init_ADCWheel()
 * �÷���Init_ADCWheel();
 * ----------------------------
 * ��ʼ�����ֵ��蹦��ģ��
 */
void Init_ADCWheel(void);

/*
 * ��������ADCWheel_ValueGet
 * �÷��� ADCWheel_ValueGet(&pui32ADCValue);
 * ----------------------------------------
 * ��ȡ����AD����ֵ
 */
void  ADCWheel_ValueGet(uint32_t *pui32ADCValue);


#endif
//-------------------------------------------------------------------------------------------------
