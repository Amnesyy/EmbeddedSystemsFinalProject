#include "UART_LPC17xx.h"
#include "LPC17xx.h"                    // Device header
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "lcd_lib/Open1768_LCD.h"
#include "lcd_lib/LCD_ILI9325.h"
#include "lcd_lib/asciiLib.h"
#include "tp_lib/TP_Open1768.h"
#include "Board_Buttons.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern ARM_DRIVER_USART Driver_USART0;

void DisplayDate();

int currentXPos = 0, currentYPos = 0;

float Gamma = 1.0;


char* itoa(int value, char* result, int base)
{
		// check that the base if valid
		if (base < 2 || base > 36) { *result = '\0'; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;

		do {
			tmp_value = value;
			value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while(ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr--= *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
}


void sendString(char* string)
{
	while(*string != '\0')
	{
		while(((LPC_UART0->LSR&32)==0))
		{
		}
		LPC_UART0->THR=*string;
		string++;
	}
		LPC_UART0->THR='\n';
		LPC_UART0->THR='\r';
}

void colourScreen(void)
{
	lcdWriteIndex(ADRX_RAM);
	lcdWriteData(100);
	lcdWriteIndex(ADRY_RAM);
	lcdWriteData(200);
	lcdWriteIndex(DATA_RAM);
	
	for(int i=0; i<LCD_MAX_X; i++)
	{
		for(int j=0; j<LCD_MAX_Y; j++)
		{
			lcdWriteData(LCDBlack);
		}
	}
	
}

uint16_t gammaCorrection(uint16_t color, float gamma)
{
	color = (color > UINT16_MAX) ? UINT16_MAX : color;
	float norm = (float)color/UINT16_MAX;
	float correct = pow(norm, 1.0/gamma);
	uint16_t result = (uint16_t)(correct * UINT16_MAX);
	return result;
}

/*
uint16_t gammaCorrection(uint16_t color, double gamma)
{
	uint16_t red = color & 1111100000000000;
	red = red >>12;
	uint16_t green = color & 11111100000;
	green = green >> 6;
	char buffer[100];
	uint16_t blue = color & 11111;
	int gred = 31.0 * pow(((double)red/31.0), 1.0/gamma);
	int ggreen= 63.0 * pow(((double)green/63.0), 1.0/gamma);
	int gblue = 31.0 * pow(((double)blue/31.0), 1.0/gamma);
	uint16_t gcolor = (gred << 11) + (ggreen << 5) + gblue;
	return gcolor;
}
*/

void drawRectangle(int x_s, int y_s, int x, int y, uint16_t color)
{
	for(int i=x_s; i<x_s+x; i++)
	{
		for(int j=y_s; j<y_s+y; j++)
		{
			lcdWriteReg(ADRX_RAM, i);
			lcdWriteReg(ADRY_RAM, j);
			lcdWriteReg(DATA_RAM, color);
		}
	}	
}


/*void drawColorfullPanel()
{
	uint16_t colors[13] = {LCDWhite, LCDBlack, LCDGrey, LCDBlue, LCDBlueSea, LCDPastelBlue, LCDViolet, LCDRed, LCDGinger, LCDGreen, LCDCyan, LCDYellow};    
	int x=0, y=48, c=0;
	while(x<LCD_MAX_X)
	{
		while(y<LCD_MAX_Y)
		{
			drawRectangle(x, y, 24, 24, colors[c%13]);
			y+=24; c++;
		}
		x+=24;
		y=48;
	}
	
}*/

void drawColorfullPanel()
{
	lcdWriteIndex(ADRX_RAM);
	lcdWriteData(LCD_MAX_X);
	lcdWriteIndex(ADRY_RAM);
	lcdWriteData(48);
	lcdWriteIndex(DATA_RAM);
	uint16_t color = 0x001F;
	
	for(int i=0; i<LCD_MAX_X; i++)
	{
		for(int j=48; j<LCD_MAX_Y - 1; j++)
		{
			lcdWriteData(color);
			color++;
		}
	}
	
	//int color18bit = (((color & 0xF800) << 2) | ((color & 0x07E0) << 1) | ((color & 0x1F) << 1));
	/*
	for(int j=48; j<LCD_MAX_Y; j++)
	{
		for(int i=0; i<LCD_MAX_X; i++)
		{
			//color18bit = (((color & 0xF800) << 2) | ((color & 0x07E0) << 1) | ((color & 0x1F) << 1));
			lcdWriteReg(ADRX_RAM, i);
			lcdWriteReg(ADRY_RAM, j);
			lcdWriteReg(DATA_RAM, color);
			color++;
		}
	}
	*/
}

void drawGammaPanel()
{
	lcdWriteIndex(ADRX_RAM);
	lcdWriteData(0);
	lcdWriteIndex(ADRY_RAM);
	lcdWriteData(50);
	lcdWriteIndex(DATA_RAM);
	char buffer[100];
	uint16_t color = 0x0000;
	//color = gammaCorrection(color, 1.5);
	//sendString(itoa(LCD_MAX_X, buffer, 10));
	for(int i=0; i<LCD_MAX_X; i++)
	{
		uint16_t gcolor = gammaCorrection(color, Gamma);
		for(int j=50; j<LCD_MAX_Y - 1; j++)
		{
			lcdWriteData(gcolor);
		}
		color+=273;
	}
	/*
	uint16_t color = 0x001F;
	char buffer[100];
	int color18bit = (((color & 0xF800) << 2) | ((color & 0x07E0) << 1) | ((color & 0x1F) << 1));
	for(int j=48; j<LCD_MAX_Y; j++)
	{
		for(int i=0; i<LCD_MAX_X; i++)
		{
			//color18bit = (((color & 0xF800) << 2) | ((color & 0x07E0) << 1) | ((color & 0x1F) << 1));
			lcdWriteReg(ADRX_RAM, i);
			lcdWriteReg(ADRY_RAM, j);
			//sendString("xd");
			//sendString(itoa(color18bit, buffer, 2));
			lcdWriteReg(DATA_RAM, gammaCorrection(color, 1.0));
			color++;
		}
	}
	*/
}


void setXandYPos()
{
	currentXPos+=8;
	if(currentXPos>LCD_MAX_X)
	{
		currentXPos = 0;
		currentYPos += 16;
	}
}


void lettersOnLcd(unsigned char c)
{
	unsigned char p[16];
	GetASCIICode(1, p, c);
	for (int i = 0; i<16; i++)
	{
		unsigned char d=0b10000000;
		for(int j = 0; j<8; j++)
		{
			if(p[i]&d){
				lcdWriteReg(ADRX_RAM, j+currentXPos);
				lcdWriteReg(ADRY_RAM, i+currentYPos);
				lcdWriteReg(DATA_RAM, LCDWhite);
			}
			d=d>>1;
		}
	}
	setXandYPos();
}

void DOWonLCD(char* DOW)
{
	
	
		
}

void textOnLcd(char* str);

void timeOnLcd(int hour, int min, int sec, int DOM, int Year, char* DOW, char* Month)
{
	currentXPos = 90;
	currentYPos = 0;
	while(*DOW != '\0')
	{
		lettersOnLcd(*DOW);
		DOW++;
	}

	
	char buffer[100];
	currentXPos = 90;
	currentYPos = 16;
	textOnLcd(itoa(DOM, buffer, 10));
	lettersOnLcd(' ');
	while(*Month != '\0')
	{
		lettersOnLcd(*Month);
		Month++;
	}
	lettersOnLcd(' ');
	textOnLcd(itoa(Year, buffer, 10));
	currentXPos = 90;
	currentYPos = 32;
	//char* hourS = itoa(hour, bufferH, 10);
	drawRectangle(80,32,80,16,LCDBlack);
	textOnLcd(itoa(hour, buffer, 10));
	lettersOnLcd(':');
	textOnLcd(itoa(min, buffer, 10));
	lettersOnLcd(':');
	textOnLcd(itoa(sec, buffer, 10));
	//char* minS = itoa(min, bufferM, 10);
	//char* secS = itoa(sec, bufferS, 10);
	/*while(*hourS != '\0')
	{
		lettersOnLcd(*hourS);
	}
	lettersOnLcd(':');
	while(*minS != '\0')
	{
		lettersOnLcd(*minS);
	}
	lettersOnLcd(':');

	while(*secS != '\0')
	{
		lettersOnLcd(*secS);
	}*/
}


void textOnLcd(char* string)
{
	while(*string != '\0')
	{
		lettersOnLcd(*string);
		string++;
	}
}

void Setup()
{
	
	PIN_Configure(0, 2, 1, 2, 0);
	PIN_Configure(0, 3, 1, 2, 0);
	LPC_TIM0->PR=25000;
	LPC_TIM0->MR0=500;
	LPC_TIM0->MCR=3;
	LPC_TIM0->TCR=1;
	
	LPC_UART0->LCR=3|(1<<7);
	LPC_UART0->DLL=27;
	LPC_UART0->DLM=0;
	LPC_UART0->LCR=3;
	
	lcdConfiguration();
	init_ILI9325();
	
	LPC_RTC->CCR=1;
	LPC_RTC->CIIR=0b00000001;
	
	NVIC_EnableIRQ(RTC_IRQn);
	Buttons_Initialize();
	
	//touchpanelInit();
}

void RTC_IRQHandler(void)
{
	LPC_RTC->ILR=0b01;
	DisplayDate();
}

int getSecFromCTIME(int consTime)
{
	return consTime&0b111111;
}

int getMinFromCTIME(int consTime)
{
	consTime = consTime&0b11111100000000;
	return consTime>>8;
}

int getHourFromCTIME(int consTime)
{
	consTime = consTime&0b111110000000000000000;
	return consTime>>17;
}

char* getDOWFromCtime(int consTime)
{
	consTime = consTime&0b111000000000000000000000000;
	consTime=consTime>>24;
	if(consTime==0) return "Monday";
	if(consTime==1) return "Tuesday";
	if(consTime==2) return "Wednesday";
	if(consTime==3) return "Thursday";
	if(consTime==4) return "Friday";
	if(consTime==5) return "Saturday";
	if(consTime==6) return "Sunday";
	return "Error: non-existing day";
}

char* getMonthFromCtime(int consTime)
{
	consTime = consTime&0b111100000000;
	consTime=consTime>>8;
	if(consTime==1) return "January";
	if(consTime==2) return "February";
	if(consTime==3) return "March";
	if(consTime==4) return "April";
	if(consTime==5) return "May";
	if(consTime==6) return "June";
	if(consTime==7) return "July";
	if(consTime==8) return "August";
	if(consTime==9) return "September";
	if(consTime==10) return "October";
	if(consTime==11) return "November";
	if(consTime==12) return "December";
	return "Error:non-existingMonth";
}

int getDOMFromCtime(int consTime)
{
	consTime = consTime&0b11111;
	return consTime;
}

int getYearFromCtime(int consTime)
{
	consTime = consTime&0b1111111111110000000000000000;
	return consTime>>16;
}

void DisplayDate()
{
	char buffer[100];
	int ConsTime = LPC_RTC->CTIME0;
	int secVal = getSecFromCTIME(ConsTime);
	//textOnLcd(itoa(secVal, buffer, 10));
	int minVal = getMinFromCTIME(ConsTime);
	//textOnLcd(itoa(minVal, buffer, 10));
	int hourVal = getHourFromCTIME(ConsTime);
	//textOnLcd(itoa(hourVal, buffer, 10));
	char* DOW = getDOWFromCtime(ConsTime);
	int ConsTime1 = LPC_RTC->CTIME1;
	char* Month = getMonthFromCtime(ConsTime1);
	int DOM = getDOMFromCtime(ConsTime1);
	int year = getYearFromCtime(ConsTime1);
	//textOnLcd(DOW);
	timeOnLcd(hourVal, minVal, secVal, DOM, year, DOW, Month);//cosik nie dziala - zdebugowac w wolnej chwili
	
}

void DisplayGamma()
{
	currentXPos = 10; 
	currentYPos = 0;
	textOnLcd("Gamma:");
	currentXPos = 10;
	currentYPos = 16;
	char buffer[100];
	drawRectangle(10,16,40,16,LCDBlack);
	textOnLcd(itoa(Gamma, buffer, 10));
	textOnLcd(".");
	if(Gamma >= 1)
	{
		int flGamma = (Gamma - 1.0)*10;
		textOnLcd(itoa(flGamma, buffer, 10));
	}
	else
	{
		int flGamma = Gamma * 10;
		textOnLcd(itoa(flGamma, buffer, 10));
	}
}


void changeGamma()
{
	int key = Buttons_GetState();
	int key1 = key & 0x1;
	int key2 = key & 0x2;
	if(Gamma>=0.1 && Gamma<=1.9)
	{
		if(key2)
		{
			Gamma = Gamma - 0.1;
			drawGammaPanel();
			DisplayGamma();
	
		}
		else if (key1)
		{
			Gamma = Gamma + 0.1;
			drawGammaPanel();
			DisplayGamma();
		}
	}
	else if(Gamma > 1.0)
	{
			Gamma = 1.9;
	}
	else
	{
			Gamma = 0.1;
	}
}

int main(void)
{
	Setup();
	colourScreen();
	sendString("xdxd");
	//drawColorfullPanel();
	drawGammaPanel();
	DisplayDate();
	DisplayGamma();
	char buffer[100];
	uint16_t reg = lcdReadReg(OSCIL_ON);
	sendString(itoa(reg, buffer, 16));
	if(reg==0x8989)
		sendString("SDD1289");
	else if(reg==0x9325)
		sendString("ILI9325");
	else if (reg == 0x8999)
		sendString("SSD1298");    
	else if(reg==0x9328)
		sendString("ILI9328");
	else 
		sendString("zaden");
	while(1)
	{
	changeGamma();
	}
}
