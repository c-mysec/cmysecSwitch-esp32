/*
 * RfDeviceControl.h
 *
 *  Created on: Jan 23, 2022
 *      Author: clovis
 */

#ifndef DEVICEHUB_H_
#define DEVICEHUB_H_
#include "globals.h"
#include "RfDeviceControl.h"

void deviceHubSetup();
bool deviceHubLoop();
void buttonRfPressed(uint8_t buttonRead);
bool controlStatusChanged(char * controlName, uint32_t state);

extern uint8_t mapButtonRead;
extern long mapButtonReadTime;

#endif /* DEVICEHUB_H_ */
