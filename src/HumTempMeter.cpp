/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "chip.h"
#include "board.h"
#include <cr_section_macros.h>

// TODO: insert other include files here
#include "inc/simpleI2Cdriver.h"
#include "inc/LCDdriver.h"
#include <stdlib.h>
// TODO: insert other definitions and declarations here

int main(void) {

	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Clear activity LED */
	Board_LED_Set(0, false);

	lcdManualInitialization();

	setupI2CGPIO();
	setupI2C();

	char celsius[3] = " C";
	char humidity[3] = " H";

	for(;;){
		measurementRequest();
		dataFetch();

		char temperBuffer[4] = {0};
		itoa(getTemperatureData(), temperBuffer, 10);

		char humBuffer[4] = {0};
		itoa(getHumidityData(), humBuffer, 10);

		setDDRAMaddress(6);
		SendDataToLCD(temperBuffer);
		SendDataToLCD(celsius);

		setDDRAMaddress(46);
		SendDataToLCD(humBuffer);
		SendDataToLCD(humidity);
	}
}
