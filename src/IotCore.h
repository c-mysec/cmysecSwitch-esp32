/*
 * IotCore.h
 *
 *  Created on: Sep 29, 2021
 *      Author: clovis
 */

#ifndef IOTCORE_H_
#define IOTCORE_H_
#include "globals.h"
void setupIotCore();
void loopIotCore();
bool sendRelayChangeMessageIotCore(const char * name, uint8_t state);


extern const char IOT_CERT_CA_PATH[];
extern const char IOT_CERT_PATH[];
extern const char IOT_PRIVKEY_PATH[];
#endif /* IOTCORE_H_ */
