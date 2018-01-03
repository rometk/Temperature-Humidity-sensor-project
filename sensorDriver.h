/*
 * simpleI2Cdriver.h
 *
 *  Created on: 6.12.2017
 *      Author: rometk
 */
#ifndef SIMPLEI2CDRIVER_H_
#define SIMPLEI2CDRIVER_H_

#include <stdint.h>
#include "chip.h"
#include "board.h"

void setupI2C();

void setupI2CGPIO();

void measurementRequest();

void dataFetch();

void SDAbusErrorCheck();

int getHumidityData();

int getTemperatureData();


#endif /* SIMPLEI2CDRIVER_H_ */


