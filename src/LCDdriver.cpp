/*
 * LCDdriver.c
 *
 *  Created on: 17.12.2017
 *      Author: rometk
 */

#include "inc/LCDdriver.h"

/*Control pins*/
volatile uint32_t* confRS 		= &LPC_IOCON->PIO[0][24];
volatile uint32_t* confRW		= &LPC_IOCON->PIO[1][0];
volatile uint32_t* confE		= &LPC_IOCON->PIO[0][27];

/*Data bus pins*/
volatile uint32_t* confDB0		= &LPC_IOCON->PIO[1][10];
volatile uint32_t* confDB1 		= &LPC_IOCON->PIO[0][12];
volatile uint32_t* confDB2 		= &LPC_IOCON->PIO[0][29];
volatile uint32_t* confDB3		= &LPC_IOCON->PIO[0][9];
volatile uint32_t* confDB4 		= &LPC_IOCON->PIO[0][10];
volatile uint32_t* confDB5		= &LPC_IOCON->PIO[0][28];
volatile uint32_t* confDB6		= &LPC_IOCON->PIO[1][3];
volatile uint32_t* confDB7		= &LPC_IOCON->PIO[0][0];

/*Control pins*/
volatile uint8_t* setRS		= &LPC_GPIO->B[0][24];
volatile uint8_t* setRW		= &LPC_GPIO->B[1][0];
volatile uint8_t* setE		= &LPC_GPIO->B[0][27];

/*Data bus pins*/
volatile uint8_t* setDB0	= &LPC_GPIO->B[1][10];
volatile uint8_t* setDB1	= &LPC_GPIO->B[0][12];
volatile uint8_t* setDB2	= &LPC_GPIO->B[0][29];
volatile uint8_t* setDB3	= &LPC_GPIO->B[0][9];
volatile uint8_t* setDB4	= &LPC_GPIO->B[0][10];
volatile uint8_t* setDB5	= &LPC_GPIO->B[0][28];
volatile uint8_t* setDB6	= &LPC_GPIO->B[1][3];
volatile uint8_t* setDB7	= &LPC_GPIO->B[0][0];


enum functionValues{
	fourBit,
	eightBit,
	oneLine,
	twoLine,
	smallFont,
	largeFont
};

enum commandValues{
	displayOff = 0x08,
	displayOn = 0x0C,
	clearDisplay = 0x01,
	setEntry = 0x06,
	DDRAMbase = 0x80
};

volatile uint8_t* dataBus[] = {setDB0, setDB1, setDB2, setDB3, setDB4, setDB5, setDB6, setDB7};
volatile uint8_t* instructionBus[] = {setRS, setRW, setE};

int done = 0;
extern "C"{
	void RIT_IRQHandler(void){
		/*Clear interrupt flag*/
		LPC_RITIMER->CTRL |= (1 << 0);

		done = 1;
	}
}

void instructionDelay(int us)
{
	done = 0;
	/*Enable counter*/
	LPC_RITIMER->CTRL |= (1 << 3);

	/*Set compare value. RIT timer is set as half the frequency of system clock.
	 * System clock = 12 MHz, RIT clock 6 MHz*/
	LPC_RITIMER->COMPVAL = 6 * us;
	LPC_RITIMER->COMPVAL_H = 0;

	while(done == 0);

	/*Disable counter*/
	LPC_RITIMER->CTRL &= ~(1 << 3);
	/*Initialize counter*/
	LPC_RITIMER->COUNTER = 0;
}

void initializeRIT(){
	/*Enable clock to RIT timer*/
	LPC_SYSCTL->SYSAHBCLKCTRL[1] |= (1 << 1);

	/*Clear the reset value to RIT*/
	LPC_SYSCTL->PRESETCTRL[1] &= ~(1 << 1);

	/*Initialize counter to 0 when reaching compare value*/
	LPC_RITIMER->CTRL |= (1 << 1);

	NVIC_SetPriority(RITIMER_IRQn, 3);
	NVIC_EnableIRQ(RITIMER_IRQn);
}

extern "C"{
	void MRT_IRQHandler(void){
		/*Reset enable signal*/
		*setE = false;
		/*Write 1 to INTFLAG bit in STAT register, clearing the interrupt flag*/
		LPC_MRT->CHANNEL[0].STAT |= (1 << 0);
	}
}

void givePulse(){
	/*Set enable signal*/
	*setE = true;
	/*Set MRT to count down from 1 which generates interrupt*/
	LPC_MRT->CHANNEL[0].INTVAL = 1;
}

void initializeMRT(){
	/*Enable clock to MRT timer*/
	LPC_SYSCTL->SYSAHBCLKCTRL[1] |= (1 << 0);

	/*Clear the reset value to MRT*/
	LPC_SYSCTL->PRESETCTRL[1] &= ~(1 << 0);

	/*Set as one-shot interrupt mode*/
	LPC_MRT->CHANNEL[0].CTRL |=	(1 << 1);

	/*Enable MRT interrupt*/
	LPC_MRT->CHANNEL[0].CTRL |= (1 << 0);

	NVIC_SetPriority(MRT_IRQn , 3);
	NVIC_EnableIRQ(MRT_IRQn);
}

void setupLCDGPIO(){
	initializeRIT();
	initializeMRT();

	/*Setup arrays*/
	volatile uint32_t* confPins[] = {confRS, confRW, confE, confDB0, confDB1, confDB2, confDB3, confDB4, confDB5, confDB6, confDB7};
	/*Arrays of pin positions*/
	volatile uint32_t pinsPort0[] = {1 << 24, 1 << 27, 1 << 12, 1 << 29, 1 << 9, 1 << 10, 1 << 28, 1 << 0};
	volatile uint32_t pinsPort1[] = {1 << 0, 1 << 10, 1 << 3};

	/*Enable clock to GPIO ports 0 and 1*/
	LPC_SYSCTL->SYSAHBCLKCTRL[0] |= (1 << 14) | (1 << 15);

	/*Set pins as output for port 0*/
	for(unsigned int i = 0; i < sizeof(pinsPort0)/sizeof(*pinsPort0); i++){
		LPC_GPIO->DIR[0] |= pinsPort0[i];
	}

	/*Set pins as output for port 1*/
	for(unsigned int i = 0; i < sizeof(pinsPort1)/sizeof(*pinsPort1); i++){
		LPC_GPIO->DIR[1] |= pinsPort1[i];
	}

	/*Enable clock to IOCON block*/
	LPC_SYSCTL->SYSAHBCLKCTRL[0] |= (1 << 13);

	/*Configure pins as digital and set no additional pin functions*/
	for(unsigned int i = 0; i < sizeof(confPins)/sizeof(*confPins); i++){
		*confPins[i] = (IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	}

	/*Disable clock to IOCON block to save power*/
	LPC_SYSCTL->SYSAHBCLKCTRL[0] &= ~(1 << 13);
}

/*Set all pins to low*/
void initializePins(){
	for(unsigned int i = 0; i < sizeof(instructionBus) / sizeof(instructionBus[0]); i++){
		*instructionBus[i] = false;
	}

	for(unsigned int i = 0; i < sizeof(dataBus) / sizeof(dataBus[0]); i++){
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 10, false);
		*dataBus[i] = false;
	}
}

void LCDfunctionSet(int interfaceType, int lineNumber, int fontType){

	if(fontType == smallFont){
		 /*Set display font to 5 x 8 dot format*/
		*setDB2 = false;
	}else if(fontType == largeFont){
		/*Set display font to 5 x 11 dot format*/
		*setDB2 = true;
	}

	if(lineNumber == oneLine){
		/*Set display line number to 1*/
		*setDB3 = false;
	}else if(lineNumber == twoLine){
		/*Set display line number to 2*/
		*setDB3 = true;
	}

	if(interfaceType == eightBit){
		/*Set MPU interface to 8-bit mode*/
		*setDB4 = true;
	}else if(interfaceType == fourBit){
		/*Set MPU interface to 4-bit mode*/
		*setDB4 = false;
	}

	/*Send functions*/
	*setDB5 = true;

	givePulse();

	instructionDelay(100);
}

void setLCDDisplayOnOff(int onOff){

	for(unsigned int i = 0; i < sizeof(dataBus) / sizeof(dataBus[0]); i++){
		if((onOff & (1 << i)) == (1 << i)){
			*dataBus[i] = true;
		}else{
			*dataBus[i] = false;
		}
	}

	givePulse();

	instructionDelay(100);
}

void displayClear(){

	for(unsigned int i = 0; i < sizeof(dataBus) / sizeof(dataBus[0]); i++){
		if((clearDisplay & (1 << i)) == (1 << i)){
			*dataBus[i] = true;
		}else{
			*dataBus[i] = false;
		}
	}

	givePulse();

	instructionDelay(2000);
}

void setEntryMode(){

	for(unsigned int i = 0; i < sizeof(dataBus) / sizeof(dataBus[0]); i++){
		if((setEntry & (1 << i)) == (1 << i)){
			*dataBus[i] = true;
		}else{
			*dataBus[i] = false;
		}
	}

	givePulse();

	instructionDelay(100);
}

void setDDRAMaddress(int offset){
	int ddramAddress = 0;
	ddramAddress |= DDRAMbase | offset;

	/*Write to instruction register*/
	*setRS = false;
	*setRW = false;

	for(unsigned int i = 0; i < sizeof(dataBus) / sizeof(dataBus[0]); i++){
		if((ddramAddress & (1 << i)) == (1 << i)){
			*dataBus[i] = true;
		}else{
			*dataBus[i] = false;
		}
	}

	givePulse();

	instructionDelay(100);
}

void SendDataToLCD(char* buffer){

	/*Write to data register*/
	*setRS = true;
	*setRW = false;

	int i = 0;
	while(buffer[i] != 0){

		for(unsigned int j = 0; j < sizeof(dataBus) / sizeof(dataBus[0]); j++){
			if((buffer[i] & (1 << j)) == (1 << j)){
				*dataBus[j] = true;
			}else{
				*dataBus[j] = false;
			}
		}

		i++;

		givePulse();

		instructionDelay(20000);
	}
}

void lcdManualInitialization(){
	setupLCDGPIO();
	initializePins();

	/*Wait at least 40ms after power on*/

	for(int i = 0; i < 2; i++) instructionDelay(65535);

	LCDfunctionSet(eightBit, oneLine, smallFont);
	instructionDelay(5000);

	LCDfunctionSet(eightBit, oneLine, smallFont);
	instructionDelay(100);

	LCDfunctionSet(eightBit, oneLine, smallFont);
	instructionDelay(100);

	LCDfunctionSet(eightBit, twoLine, smallFont);
	instructionDelay(100);

	setLCDDisplayOnOff(displayOn);

	displayClear();

	setEntryMode();
}
