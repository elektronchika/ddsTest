/*
===============================================================================
 Name        : main.c
 Author      : elektronchika
 Version     : v0.01
 Copyright   : Copyright (C) 
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC122x.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

#include <stdio.h>
//#include <math.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

void init();
void initSpi();
//void initDAC();
//void dacWrite(uint32_t writeData);
void ddsWrite(uint16_t writeData);
void initDDS();
void setFreq0(uint64_t freq0);

int i;

const uint64_t MCLK = 25000000;
const uint32_t ddsBits = 28;
const uint32_t ddsCombs = 268435456;

int main(void) {
	
	// TODO: insert code here

	// Enter an infinite loop, just incrementing a counter
	//volatile static int i = 0 ;
	init();
	initSpi();
	initDDS();
	//initDAC();
	setFreq0(10000);
	while(1) {
		i++ ;
		//initDDS();
	}
	return 0 ;
}

void init(){
	// Enable clock
	LPC_SYSCON->SYSAHBCLKCTRL |= 0xE001081F;  // Enable clock for: Sys, Rom, Ram, FlashReg, FlashArray, SSP, IOCON, GPIO0, GPIO1, GPIO2
	// init LED
	LPC_GPIO0->MASK = 0xFFFE << 7;
	LPC_GPIO0->DIR = 0xFFFF;   // Set PIO0_7 (red LED) as output
	LPC_GPIO0->MASK = !0x8008;
	LPC_GPIO0->DIR = 0x8008;  // Set PIO0_3 (dac CS) and PIO0_15 (dds CS) as output
	LPC_GPIO0->SET = 0x8008;  // set both CS
}

void initSpi(){
	// initialize SSP ports
	LPC_IOCON->PIO0_14 |= 0x00000082;  // SCK
	LPC_IOCON->PIO0_15 |= 0x00000082;  // 80 = GPIO, (old 82 = SSEL)
	LPC_IOCON->PIO0_16 |= 0x00000082;  // MISO
	LPC_IOCON->PIO0_17 |= 0x00000082;  // MOSI
	//LPC_IOCON->PIO0_3  |= 0x00000080;  // dual dac CS
	// Initialize SSP module, looking for 200kHz at SCLK
	LPC_SYSCON->SSPCLKDIV = 0x3c;
	LPC_SSP->CR0 = 0x000000cf;  // Serial clock rate = ??, CPHA = 1, CPOL = 0, SPI, 16 bits
	LPC_SSP->CPSR = 0x00000002; // Clock prescaler = 2
	LPC_SSP->CR1 = 0x00000002;  // SSP is master
}

void ddsWrite(uint16_t writeData){
	// CS low
	//LPC_GPIO0->MASK = !0x8000;
	//LPC_GPIO0->CLR = 0x8000;
	LPC_SSP->DR = writeData;
	while((LPC_SSP->SR & (0x10|0x2)) != 0x2);
	// CS high
	//LPC_GPIO0->MASK = !0x8000;
	//LPC_GPIO0->SET = 0x8000;
}

/*void dacWrite(uint32_t writeData){
	// CS low
	LPC_GPIO0->MASK = !0x0008;
	LPC_GPIO0->CLR = 0x0008;
	LPC_SSP->DR = (0xff0000 & writeData) >> 16;
	while((LPC_SSP->SR & (0x10|0x2)) != 0x2);
	LPC_SSP->DR = (0xff00 & writeData) >> 8;
	while((LPC_SSP->SR & (0x10|0x2)) != 0x2);
	LPC_SSP->DR = (0x00ff & writeData);
	while((LPC_SSP->SR & (0x10|0x2)) != 0x2);
	// CS high
	LPC_GPIO0->MASK = !0x0008;
	LPC_GPIO0->SET = 0x0008;
}*/



// This function initialize AD9834 with the AN-1070 method
// with 25MHz clock, this should give 400Hz output frequency
void initDDS(){
/*	ddsWrite(0x2100);   // ctrl reg
	ddsWrite(0X4ac7);   // freq reg 0 lsb
	ddsWrite(0X8000);   // freq reg 0 msb
	ddsWrite(0X50c7);   // freq reg 1 lsb
	ddsWrite(0X8000);   // freq reg 1 msb
	ddsWrite(0XC000);   // phase reg 0
	ddsWrite(0XE000);   // phase reg 1
	ddsWrite(0x0000);   // ctrl reg // working with 25MHz MCLK
*/
/*	ddsWrite(0x0100);   // lsb only
	ddsWrite(0x50c7);   // reg0
	ddsWrite(0x90c7);   // reg1
	//ddsWrite(0x1100);   // msb only
	//ddsWrite(0x4000);   // reg0
	//ddsWrite(0x8000);   // reg1
	ddsWrite(0xc000);   // phase0
	ddsWrite(0xe000);   // phase1
	ddsWrite(0x0000);   // control, freq0
*/

	ddsWrite(0x2100);   // ctrl reg
	ddsWrite(0X50c7);   // freq reg 0 lsb
	ddsWrite(0X4000);   // freq reg 0 msb
	ddsWrite(0X90c7);   // freq reg 1 lsb
	ddsWrite(0X8000);   // freq reg 1 msb
	ddsWrite(0XC000);   // phase reg 0
	ddsWrite(0XE000);   // phase reg 1
	ddsWrite(0x2000);   // ctrl reg // working with 25MHz MCLK
}

/*void setFreq0(uint64_t freq0){
	uint64_t ftw0;
	uint16_t low16, high16;
	ftw0 = (freq0<<28)/MCLK;
	low16 = 0x8000 + ftw0*0x3fff;
	high16 = 0x8000 + (ftw0>>14)*0x3fff;
	ddsWrite(0x2100);
	ddsWrite(low16);
	ddsWrite(high16);
	ddsWrite(0x2000);
}*/

void setFreq0(uint64_t freq0){
	uint64_t ftw0;
	uint16_t low16, high16;
	ftw0 = (freq0<<28)/MCLK;
	low16 = ftw0 & 0x3fff;
	low16 = low16 + 0x4000;
	high16 = (ftw0>>14);
	high16 &= 0x3fff;
	high16 = high16 + 0x4000;
//	ddsWrite(0x2100);
//	ddsWrite(low16);
//	ddsWrite(high16);
//	ddsWrite(0x2000);

	//
	ddsWrite(0x2100);   // ctrl reg
//	ddsWrite(0X4ac7);   // freq reg 0 lsb
//	ddsWrite(0X8000);   // freq reg 0 msb
	ddsWrite(low16);    // freq reg 1 lsb
	ddsWrite(high16);   // freq reg 1 msb
//	ddsWrite(0XC000);   // phase reg 0
//	ddsWrite(0XE000);   // phase reg 1
	ddsWrite(0x2000);   // ctrl reg // working with 25MHz MCLK
}

/*void initDAC(){
	dacWrite(0x00300003); // set LDAC function to don't care
	dacWrite(0x00380001); // enable internal reference
	dacWrite(0x0018ffff); // write to dac A
	dacWrite(0x00193fff); // write to dac B
}*/
