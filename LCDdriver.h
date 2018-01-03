/*
 * LCDdriver.h
 *
 *  Created on: 18.12.2017
 *      Author: rometk
 */

#ifndef LCDDRIVER_H_
#define LCDDRIVER_H_

#endif /* LCDDRIVER_H_ */

#include "chip.h"
#include <stdlib.h>

void LCDfunctionSet(int interfaceType, int lineNumber, int fontType);
/*onOff == 1: display on, onOff == 0: display off*/
void setLCDDisplayOnOff(int onOff);
void SendDataToLCD(char* buffer);

void setDDRAMaddress(int offSet);

/*Set all pins to low*/
void initializePins();

void instructionDelay(int microSeconds);

/*When electrical characteristics of the modules automatic initialization aren't met,
 *manual initialization needs to be performed*/
void lcdManualInitialization();
