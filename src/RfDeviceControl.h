/*
 * RfDeviceControl.h
 *
 *  Created on: Jan 23, 2022
 *      Author: clovis
 */

#ifndef RFDEVICECONTROL_H_
#define RFDEVICECONTROL_H_
#include "globals.h"

extern uint32_t rFDeviceControlLastCode;
extern uint8_t rFDeviceControlLastLen;
extern uint8_t rFDeviceControlLastProto;

void rfDeviceControlSetup();
uint8_t rfDeviceControlReceive();
bool rfDeviceControlSend(uint8_t actuator, uint8_t command, bool send);
bool rfDeviceControlSend(const char * name, uint8_t command, bool send);
void rfDeviceControlTestSend(int protocol, int bitlen, long code);
#endif /* RFDEVICECONTROL_H_ */
